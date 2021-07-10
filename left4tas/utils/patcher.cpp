// C++
// Patcher

#include "patcher.h"

CPatcher::CPatcher() : m_pAddress(NULL), m_pOriginalBytes(NULL), m_pPatchedBytes(NULL), m_nPatchLength(0)
{
}

void CPatcher::Init(void *pAddress, void *pPatchedBytes, const int nPatchLength)
{
	m_pOriginalBytes = (PBYTE)calloc(nPatchLength, sizeof(BYTE));
	m_pPatchedBytes = (PBYTE)calloc(nPatchLength, sizeof(BYTE));

	if (m_pOriginalBytes == NULL || m_pPatchedBytes == NULL)
		return;

	m_pAddress = (PBYTE)pAddress;
	m_nPatchLength = nPatchLength;

	memcpy(m_pOriginalBytes, pAddress, nPatchLength);
	memcpy(m_pPatchedBytes, pPatchedBytes, nPatchLength);
}

void CPatcher::Remove()
{
	if (m_pOriginalBytes)
		delete m_pOriginalBytes;

	if (m_pPatchedBytes)
		delete m_pPatchedBytes;

	m_pOriginalBytes = m_pPatchedBytes = NULL;
}

bool CPatcher::Patch()
{
	if (m_pOriginalBytes == NULL || m_pPatchedBytes == NULL)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pAddress, m_nPatchLength, PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pAddress, m_pPatchedBytes, m_nPatchLength);

	VirtualProtect(m_pAddress, m_nPatchLength, dwProtection, &dwProtection);

	return true;
}

bool CPatcher::Unpatch()
{
	if (m_pOriginalBytes == NULL || m_pPatchedBytes == NULL)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pAddress, m_nPatchLength, PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pAddress, m_pOriginalBytes, m_nPatchLength);

	VirtualProtect(m_pAddress, m_nPatchLength, dwProtection, &dwProtection);

	return true;
}