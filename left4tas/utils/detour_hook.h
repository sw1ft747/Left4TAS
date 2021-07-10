// C++
// Detour Hook

#pragma once

#include <Windows.h>
#include <string.h>

#define DETOUR_HOOK(name) CDetour name;

#define INIT_DETOUR(name, originalFunction, hookFunction, returnAddress) name.Init(originalFunction, hookFunction, returnAddress);
#define INIT_DETOUR_ARBITRARY_LENGTH(name, originalFunction, hookFunction, length, returnAddress) name.Init(originalFunction, hookFunction, length, returnAddress);
#define REMOVE_DETOUR(name) name.Remove();

#define DETOUR_HOOK_FUNC(name) name.HookFunction();
#define DETOUR_UNHOOK_FUNC(name) name.UnhookFunction();

class CDetour
{
public:
	CDetour();

	void Init(void *pOriginalFunction, void *pDetourFunction, DWORD *dwReturnAddress);

	void Init(void *pOriginalFunction, void *pDetourFunction, const DWORD nLength, DWORD *dwReturnAddress);
	
	void Remove();

	bool HookFunction();
	
	bool UnhookFunction();

private:
	void *m_pOriginalFunction;
	void *m_pDetourFunction;

	BYTE *m_pOriginalBytes;
	BYTE *m_pPatchedBytes;

	DWORD m_nLength;
};