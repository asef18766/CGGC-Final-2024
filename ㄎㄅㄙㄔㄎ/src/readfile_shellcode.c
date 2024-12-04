#include <windows.h>
#include <stdio.h>
#include "GetProcAddressWithHash.h"
#define BUFFERSIZE 50

void dead()
{
    void* a = CreateFileA;
    void* b = ReadFileEx;
}

void get_flag(char *ReadBuffer)
{
    HANDLE hFile; 
    DWORD  dwBytesRead = 0;
    OVERLAPPED ol = {0};
    char flagstr[] = "flag";

    HANDLE (*CreateFileA_) (LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE) = GetProcAddressWithHashInIAT(CreateFileA_HASH);

    hFile = CreateFileA_(flagstr,               // file to open
                       GENERIC_READ,          // open for reading
                       FILE_SHARE_READ,       // share for reading
                       NULL,                  // default security
                       OPEN_EXISTING,         // existing file only
                       FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, // normal file
                       NULL);                 // no attr. template

    // Read one character less than the buffer size to save room for
    // the terminating NULL character. 

    BOOL (*ReadFileEx_)(HANDLE, LPVOID, DWORD, LPOVERLAPPED, LPOVERLAPPED_COMPLETION_ROUTINE) = GetProcAddressWithHashInIAT(ReadFileEx_HASH);
    ReadFileEx_(hFile, ReadBuffer, BUFFERSIZE-1, &ol, NULL);
    for (int i = 0; i != BUFFERSIZE-1; ++i)
        ReadBuffer[i] ^= 0x87;
}


int main()
{
    
    char   ReadBuffer[BUFFERSIZE] = {0};
    void (*shellcode)(void*) = VirtualAlloc(NULL, 1024, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memset(shellcode, 0x90, 0x1000);
	FILE *sc = fopen("sc", "rb");
    fread(shellcode, 1068, 1, sc);
    shellcode(ReadBuffer);
    //get_flag(ReadBuffer);
    printf("got %s\n", ReadBuffer);

    
    return 0;

}