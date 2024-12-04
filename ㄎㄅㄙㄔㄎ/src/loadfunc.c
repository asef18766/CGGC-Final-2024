#include "GetProcAddressWithHash.h"

/*
UINT32 HashFunc(char *modName, char *funcName)
{
    // Calculate the module hash
    UINT32 dwModuleHash = 0;
    UINT32 dwFunctionHash = 0;


    for (char *pTempChar = modName; *pTempChar != 0; ++pTempChar)
    {
        dwModuleHash = ROTR32( dwModuleHash, 13 );
        if ( *pTempChar >= 0x61 )
            dwModuleHash += *pTempChar - 0x20;
        else
            dwModuleHash += *pTempChar;
    }
	printf("dwModuleHash:%x\n", dwModuleHash);
    dwFunctionHash = 0;
	for (char *pTempChar = funcName; *pTempChar !=0; ++pTempChar)
	{
		dwFunctionHash = ROTR32( dwFunctionHash, 13 );
		dwFunctionHash += *pTempChar;
	}
	dwFunctionHash += dwModuleHash;
    return dwFunctionHash;
}
*/

//HMODULE GetProcAddressWithHashInIAT(char *funcName)
HMODULE GetProcAddressWithHashInIAT(DWORD dwModuleFunctionHash)
{
	PPEB PebAddress;
	PMY_PEB_LDR_DATA pLdr;
	PMY_LDR_DATA_TABLE_ENTRY pDataTableEntry;
	PVOID pModuleBase;
	PIMAGE_NT_HEADERS pNTHeader;
	DWORD dwImportDirRVA;

	PLIST_ENTRY pNextModule;
	DWORD dwNumFunctions;
	USHORT usOrdinalTableIndex;
	PDWORD pdwFunctionNameBase;
	PCSTR pFunctionName;

	DWORD dwModuleHash  = 0;
	DWORD dwFunctionHash  = 0;
	PCSTR pTempChar;
	DWORD i;

	char *importDllName = NULL;
	PebAddress = (PPEB)__readgsqword(0x60);

	pLdr = (PMY_PEB_LDR_DATA)PebAddress->Ldr;
	pNextModule = pLdr->InLoadOrderModuleList.Flink;
	pDataTableEntry = (PMY_LDR_DATA_TABLE_ENTRY)pNextModule;

	while (pDataTableEntry->DllBase != NULL)
	{
		pModuleBase = pDataTableEntry->DllBase;
		pNTHeader = (PIMAGE_NT_HEADERS)((ULONG_PTR)pModuleBase + ((PIMAGE_DOS_HEADER)pModuleBase)->e_lfanew);
		
		dwImportDirRVA = pNTHeader->OptionalHeader.DataDirectory[1].VirtualAddress;
		if (pNTHeader->FileHeader.Characteristics & IMAGE_FILE_DLL)
			continue;
		// Get the next loaded module entry
		pDataTableEntry = (PMY_LDR_DATA_TABLE_ENTRY)pDataTableEntry->InLoadOrderLinks.Flink;

		// If the current module does not export any functions, move on to the next module.
		if (dwImportDirRVA == 0)
		{
			continue;
		}
		for (
			IMAGE_IMPORT_DESCRIPTOR *pImportDir = ((ULONG_PTR)pModuleBase + dwImportDirRVA);
			pImportDir->Name;
			pImportDir++
		)
		{
			importDllName = (ULONG_PTR)pModuleBase + pImportDir->Name;
			//printf("got dll: %s\n", importDllName);
			dwModuleHash = 0;
		
            for (char *pTempChar = importDllName; *pTempChar != 0; ++pTempChar)
            {
                dwModuleHash = ROTR32( dwModuleHash, 13 );
                if ( *pTempChar >= 0x61 )
                    dwModuleHash += *pTempChar - 0x20;
                else
                    dwModuleHash += *pTempChar;
            }
            //printf("dwModuleHash:%x\n", dwModuleHash);

			for (
                IMAGE_THUNK_DATA64 *imageThunkData = (ULONG_PTR)pModuleBase + pImportDir->OriginalFirstThunk, 
                                   *firstThunk     = (ULONG_PTR)pModuleBase + pImportDir->FirstThunk; 
                imageThunkData->u1.AddressOfData != NULL;
                imageThunkData++, firstThunk++)
			{
				IMAGE_IMPORT_BY_NAME *impByName = (ULONG_PTR)pModuleBase + imageThunkData->u1.AddressOfData;
                char *funcName = impByName->Name;
                dwFunctionHash = 0;
                for (char *pTempChar = funcName; *pTempChar !=0; ++pTempChar)
                {
                    dwFunctionHash = ROTR32( dwFunctionHash, 13 );
                    dwFunctionHash += *pTempChar;
                }
                dwFunctionHash += dwModuleHash;
                
				//printf("%s -> %x\n", impByName->Name, dwFunctionHash);
                if ( dwFunctionHash == dwModuleFunctionHash )
                //if (strcmp(impByName->Name, funcName) == 0)
                {
                    //printf("found %s at %x\n", funcName, firstThunk->u1.AddressOfData);
                    return firstThunk->u1.AddressOfData;
                }
			}
			//puts("end");
		}
		break;
	}

	// All modules have been exhausted and the function was not found.
	return NULL;
}

/*
#include <stdio.h>

int main()
{
    printf("%x\n", HashFunc("KERNEL32.dll", "CreateFileA"));
    printf("%x\n", HashFunc("KERNEL32.dll", "ReadFileEx"));

}

*/