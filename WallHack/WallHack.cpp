// WallHack.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <windows.h>
#include <stdlib.h>

#include <d3d9.h>
#include <d3dx9.h>
 
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

bool Compare(const BYTE* pData, const BYTE* bMask, const char* szMask)
{
	for (; *szMask; ++szMask, ++pData, ++bMask)
	if (*szMask == 'x' && *pData != *bMask)   return 0;
	return (*szMask) == NULL;
}

DWORD FindPattern(DWORD dwAddress, DWORD dwLen, BYTE *bMask, char * szMask)
{
	for (DWORD i = 0; i<dwLen; i++)
	if (Compare((BYTE*)(dwAddress + i), bMask, szMask))  return (DWORD)(dwAddress + i);
	return 0;
}


DWORD addressDIP;
DWORD addressJMPBACK;

void WallHack(LPDIRECT3DDEVICE9 pDevice) {

	IDirect3DVertexBuffer9* pStreamData = NULL; 
	UINT iOffsetInBytes,iStride;  
	pDevice->GetStreamSource(0,&pStreamData,&iOffsetInBytes,&iStride); 
	
	if ((iStride == 48))
	{
		pDevice->SetRenderState(D3DRS_ZENABLE, D3DZB_FALSE);
	}
	// if (iStride == 44)
	// {
		
	// }
}

__declspec(naked) void myDIP()
{
	LPDIRECT3DDEVICE9 pDevice;

	__asm 
    {	
		mov edi, edi
		push ebp
		mov ebp, esp
		mov eax, [ebp + 0x8]
		mov pDevice, eax
		pushad
	}
	
	//MessageBoxA(0, "aaaa", "bbbbb", 0);

	WallHack(pDevice);

	__asm 
    {
		popad
		JMP addressJMPBACK 
	}
}


DWORD WINAPI nThread(LPVOID lpParameter)
{
    DWORD hD3D, adr, *vTable;
    hD3D=0;
    do {
            hD3D = (DWORD)GetModuleHandle("d3d9.dll"); // tim thu vien d3d9
            Sleep(10);
    } while(!hD3D);
	
	adr = FindPattern(hD3D, 0x128000, (PBYTE)"\xC7\x06\x00\x00\x00\x00\x89\x86\x00\x00\x00\x00\x89\x86", "xx????xx????xx");
	
	if (adr)
	{
		memcpy(&vTable, (void *)(adr + 2), 4);
		addressDIP = (DWORD)vTable[82]; // vi tri ham drawindexprimitive
		addressJMPBACK = addressDIP + 0x5; //
		DetourFunction((BYTE*)addressDIP, (BYTE*)myDIP, 5); // hook drawindexprimitive
	}

	// addressDIP = FindPattern(hD3D, 0x128000,(PBYTE)"\x8B\xFF\x55\x8B\xEC\x6A\xFF\x68\x00\x00\x00\x00\x64\xA1\x00\x00\x00\x00\x50\x83\xEC\x10\x53\x56\x57\xA1\x00\x00\x00\x00","xxxxxxxx????xx????xxxxxxxx????");
	// if(addressDIP)
	// {
	//	addressJMPBACK = addressDIP + 0x5;
	//	DetourFunction((BYTE*)addressDIP, (BYTE*)myDIP, 5);
	// }

	

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
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)nThread, NULL, NULL, NULL);
	}
    return TRUE;
}

