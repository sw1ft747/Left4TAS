// C++
// Detour Hook

#include "detour_hook.h"
#include "../libdasm/libdasm.h"

CDetour::CDetour() : m_pOriginalFunction(NULL), m_pDetourFunction(NULL), m_pOriginalBytes(NULL), m_pPatchedBytes(NULL), m_nLength(0)
{
}

void CDetour::Init(void *pOriginalFunction, void *pDetourFunction, DWORD *dwReturnAddress)
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
	m_pDetourFunction = pDetourFunction;

	m_pOriginalBytes = new BYTE[nLength];
	m_pPatchedBytes = new BYTE[nLength];

	m_nLength = nLength;

	memcpy(m_pOriginalBytes, pOriginalFunction, nLength);
	memset(m_pPatchedBytes, 0x90, nLength);

	*m_pPatchedBytes = 0xE9;
	*(PDWORD)(m_pPatchedBytes + 1) = (DWORD)pDetourFunction - (DWORD)pOriginalFunction - 5;

	*dwReturnAddress = (DWORD)pOriginalFunction + nLength;
}

void CDetour::Init(void *pOriginalFunction, void *pDetourFunction, const DWORD nLength, DWORD *dwReturnAddress)
{
	if (nLength < 5)
		return;

	m_pOriginalFunction = pOriginalFunction;
	m_pDetourFunction = pDetourFunction;
	
	m_pOriginalBytes = new BYTE[nLength];
	m_pPatchedBytes = new BYTE[nLength];

	m_nLength = nLength;

	memcpy(m_pOriginalBytes, pOriginalFunction, nLength);
	memset(m_pPatchedBytes, 0x90, nLength);

	*m_pPatchedBytes = 0xE9;
	*(PDWORD)(m_pPatchedBytes + 1) = (DWORD)pDetourFunction - (DWORD)pOriginalFunction - 5;

	*dwReturnAddress = (DWORD)pOriginalFunction + nLength;
}

void CDetour::Remove()
{
	if (m_pOriginalBytes)
		delete m_pOriginalBytes;

	if (m_pPatchedBytes)
		delete m_pPatchedBytes;

	m_pOriginalBytes = m_pPatchedBytes = NULL;
}

bool CDetour::HookFunction()
{
	if (m_pOriginalBytes == NULL || m_pPatchedBytes == NULL)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pOriginalFunction, m_nLength, PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pOriginalFunction, m_pPatchedBytes, m_nLength);

	VirtualProtect(m_pOriginalFunction, m_nLength, dwProtection, &dwProtection);

	return true;
}

bool CDetour::UnhookFunction()
{
	if (m_pOriginalBytes == NULL || m_pPatchedBytes == NULL)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pOriginalFunction, m_nLength, PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pOriginalFunction, m_pOriginalBytes, m_nLength);

	VirtualProtect(m_pOriginalFunction, m_nLength, dwProtection, &dwProtection);

	return true;
}