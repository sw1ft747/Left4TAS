// Trampoline Hook

#pragma once

#include <Windows.h>
#include <string.h>

//-----------------------------------------------------------------------------

#define TRAMPOLINE_HOOK(name) CTrampoline name

#define INIT_HOOK(name, originalFunction, hookFunction) name.Init(originalFunction, hookFunction)
#define REMOVE_HOOK(name) name.Remove()

#define HOOK_FUNC(name, trampoline, funcType) if (name.HookFunction()) trampoline = (funcType)name.GetTrampoline()
#define UNHOOK_FUNC(name) name.UnhookFunction()

#define HOOK_FUNCTION(name, originalFunction, hookFunction, trampoline, funcType) INIT_HOOK(name, originalFunction, hookFunction); HOOK_FUNC(name, trampoline, funcType)
#define UNHOOK_FUNCTION(name) UNHOOK_FUNC(name); REMOVE_HOOK(name)

//-----------------------------------------------------------------------------

class CTrampoline
{
public:
	CTrampoline();
	~CTrampoline();

	void Init(void *pOriginalFunction, void *pHookFunction);

	void Remove();

	bool HookFunction();

	bool UnhookFunction();

	void *GetTrampoline() const;

private:
	void *m_pTrampoline;
	void *m_pOriginalFunction;
	void *m_pHookFunction;

	BYTE *m_pOriginalBytes;
	BYTE *m_pPatchedBytes;

	DWORD m_nLength;
};