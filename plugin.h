#pragma once

#include <iostream>
#include <Windows.h>
#include "mm/hookmgr.h"

#define DLLATTATCH BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) \
{ \
	if (dwReason == DLL_PROCESS_ATTACH) { \
		HookMgr::Initialize(); \
		PluginAttach(hModule, dwReason, lpReserved); \
	} \
	return TRUE; \
}

void PluginAttach(HMODULE hModule, DWORD dwReason, LPVOID lpReserved);
void PluginHooks();
