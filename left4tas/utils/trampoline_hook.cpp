
// Trampoline Hook

#include "trampoline_hook.h"
#include "../libdasm/libdasm.h"

CTrampoline::CTrampoline() : m_pOriginalFunction(NULL), m_pHookFunction(NULL), m_pTrampoline(NULL), m_pOriginalBytes(NULL), m_pPatchedBytes(NULL), m_nLength(0)
{
}

CTrampoline::~CTrampoline()
{
	Remove();
}

void CTrampoline::Init(void *pOriginalFunction, void *pHookFunction)
{
	int nLength = 0;

	INSTRUCTION instruction;
	BYTE *buffer = (PBYTE)pOriginalFunction;

	do
	{
		int result = get_instruction(&instruction, buffer, MODE_32);

		buffer += result;
		nLength += result;

	} while (nLength < 5);

	m_pOriginalFunction = pOriginalFunction;
	m_pHookFunction = pHookFunction;

	m_pOriginalBytes = new BYTE[nLength];
	m_pPatchedBytes = new BYTE[nLength];

	m_nLength = nLength;

	static BYTE pCallBytes[5] = { 0xE9 };
	DWORD nNOPs = nLength - 5;

	*(PDWORD)(pCallBytes + 1) = (DWORD)pHookFunction - (DWORD)pOriginalFunction - 5;

	memcpy(m_pOriginalBytes, pOriginalFunction, nLength);
	memcpy(m_pPatchedBytes, pCallBytes, 5);

	if (nNOPs) memset((PBYTE)m_pPatchedBytes + 5, 0x90, nNOPs);
}

void CTrampoline::Remove()
{
	if (m_pOriginalBytes)
		delete[] m_pOriginalBytes;

	if (m_pPatchedBytes)
		delete[] m_pPatchedBytes;

	m_pOriginalBytes = m_pPatchedBytes = NULL;
	m_nLength = 0;
}

bool CTrampoline::HookFunction()
{
	if (m_nLength)
	{
		DWORD dwProtection;
		static BYTE pCallBytes[5] = { 0xE9 };

		m_pTrampoline = NULL;
		m_pTrampoline = VirtualAlloc(NULL, m_nLength + 5, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

		if (m_pTrampoline)
		{
			*(PDWORD)(pCallBytes + 1) = (DWORD)((PBYTE)m_pOriginalFunction + m_nLength) - (DWORD)((PBYTE)m_pTrampoline + m_nLength) - 5;

			memcpy(m_pTrampoline, m_pOriginalBytes, m_nLength);
			memcpy((PBYTE)m_pTrampoline + m_nLength, pCallBytes, 5);

			VirtualProtect(m_pOriginalFunction, m_nLength, PAGE_EXECUTE_READWRITE, &dwProtection);

			memcpy(m_pOriginalFunction, m_pPatchedBytes, m_nLength);

			VirtualProtect(m_pOriginalFunction, m_nLength, dwProtection, &dwProtection);

			return true;
		}
	}
	
	return false;
}

bool CTrampoline::UnhookFunction()
{
	if (m_pTrampoline)
		VirtualFree(m_pTrampoline, 0, MEM_RELEASE);
	else
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pOriginalFunction, m_nLength, PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pOriginalFunction, m_pOriginalBytes, m_nLength);

	VirtualProtect(m_pOriginalFunction, m_nLength, dwProtection, &dwProtection);
	
	m_pTrampoline = NULL;

	return true;
}

void *CTrampoline::GetTrampoline() const
{
	return m_pTrampoline;
}