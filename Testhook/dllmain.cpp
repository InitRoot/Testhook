// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"

#include <windows.h>
#include<iostream>
#include "detours\detours.h"
#include <ws2tcpip.h>

#pragma comment( lib, "msvcrt.lib" )
#pragma comment( lib, "detours.lib" )
#pragma comment(lib, "ws2_32.lib")
//#pragma comment( lib, "detoured.lib" )

FILE *pSendLogFile;
FILE *pRecvLogFile;

/* ########################### POINTERS ########################### */


/* send */
int (WINAPI *dsend)(SOCKET, const char*, int, int) = send;
int WINAPI mysend(SOCKET s, const char* buf, int len, int flags);

/* recv */
int (WINAPI *drecv)(SOCKET, char*, int, int) = recv;
int WINAPI myrecv(SOCKET s, char* buf, int len, int flags);

/* connect */
int (WINAPI *dconnect)(SOCKET, const struct sockaddr*, int) = connect;
int WINAPI myconnect(SOCKET s, const struct sockaddr *name, int namelen);

/* memcpy */
void *(*Real_Memcpy)(void *dest, const void *src, size_t count) = memcpy;
void *(*Real_Memcpy_add)(void *dest, const void *src, size_t count) = &memcpy;
void * Mine_Memcpy(void *dest, const void *src, size_t count);


/* ########################### FUNCTIONS ########################### */
void* Mine_Memcpy(void *dest, const void *src, size_t count) {
	HANDLE hFile;
	BOOL bErrorFlag = FALSE;
	DWORD dwBytesWritten = 0;
	char cislo[50]; // just big enough
	//sprintf(cislo, "0x%08x", Real_Memcpy_add);

	//MessageBoxA(0, cislo, cislo, 0);
	//MessageBoxA(0, (char *)src, (char *)src, 0);

	return 0x0;
}

int WINAPI mysend(SOCKET s, const char* buf, int len, int flags) {

	return dsend(s, buf, len, flags);
}

int WINAPI myrecv(SOCKET s, char* buf, int len, int flags) {

	return drecv(s, buf, len, flags);
}

int WINAPI myconnect(SOCKET s, const struct sockaddr *name, int namelen) {

	return dconnect(s, name, namelen);
}




BOOL WINAPI DllMain(HINSTANCE, DWORD dwReason, LPVOID) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		/* memcpy*/
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID &)Real_Memcpy, Mine_Memcpy);
		if (DetourTransactionCommit() == NO_ERROR) {
			OutputDebugStringA("HOOKED: memcpy succeed");
		}
				
		/* connect */
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dconnect, myconnect);
		if (DetourTransactionCommit() == NO_ERROR) { 
			OutputDebugStringA("HOOKED: connect succeed");
		}

		/* send */
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)dsend, mysend);
		if (DetourTransactionCommit() == NO_ERROR) { OutputDebugStringA("HOOKED: send succeed"); }
		/* receive */
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourAttach(&(PVOID&)drecv, myrecv);
		if (DetourTransactionCommit() == NO_ERROR) { OutputDebugStringA("HOOKED: receive succeed"); }
		break;
		

	case DLL_PROCESS_DETACH:
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID &)Real_Memcpy, Mine_Memcpy);
		DetourTransactionCommit();
		break;
	}

	return TRUE;
}

