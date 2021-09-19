// WallHack.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>

//#include <d3d9.h>
//#include <d3dx9.h>
 
//#pragma comment(lib, "d3d9.lib")
//#pragma comment(lib, "d3dx9.lib")

void *DetourFunction (BYTE *src, const BYTE *dst, const int len)
{
        BYTE *jmp = (BYTE*)malloc(len+5);
        DWORD dwBack;
 
        VirtualProtect(src, len, PAGE_EXECUTE_READWRITE, &dwBack);
        memcpy(jmp, src, len);
        jmp += len;
        jmp[0] = 0xE9;
        *(DWORD*)(jmp+1) = (DWORD)(src+len - jmp) - 5;
        src[0] = 0xE9;
        *(DWORD*)(src+1) = (DWORD)(dst - src) - 5;
        for (int i=5; i<len; i++)  
			src[i]=0x90;
        VirtualProtect(src, len, dwBack, &dwBack);
        return (jmp-len);
}

DWORD addressHookHP; // server.dll+26FE2 - 5F                    - pop edi
DWORD retHookHP; // server.dll+26FE7 - C2 0400               - ret 0004

__declspec(naked) void myHP()
{
	DWORD addressHP;

	__asm 
    {	
		mov addressHP, esi
		pushad
	}

	*(DWORD*)(addressHP) = 100;
	
	__asm 
    {
		popad
		pop edi
		mov eax, esi
		pop esi
		pop ecx
		JMP retHookHP
	}
}

DWORD initHookHP() {
	DWORD hServerDLL;
	do {
            hServerDLL = (DWORD)GetModuleHandle("server.dll"); // tim thu vien d3d9
            Sleep(10);
    } while(!hServerDLL);
	
	addressHookHP = hServerDLL + 0x26FE2; //server.dll+26FE2 - 5F                    - pop edi
	retHookHP = addressHookHP + 0x5; // C2 0400               - ret 0004
	
	DetourFunction((BYTE*)addressHookHP, (BYTE*)myHP, 5);

	return 0;
}


// DllMain
BOOL APIENTRY DllMain( HMODULE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	DisableThreadLibraryCalls(hModule);
    if ( ul_reason_for_call == DLL_PROCESS_ATTACH ) {
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)initHookHP, NULL, NULL, NULL);
		// CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)nThread, NULL, NULL, NULL);
	}
    return TRUE;
}

