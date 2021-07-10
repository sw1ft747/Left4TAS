// C++
// VTable Hook

#include "vtable_hook.h"

CVTableHook::CVTableHook() : m_pBaseClass(NULL), m_pVTable(NULL), m_pVTableOriginal(NULL), m_nFunctions(0)
{
}

void CVTableHook::Init(void *pBaseClass, const int nFunctions)
{
	if (nFunctions > 0)
	{
		m_pBaseClass = static_cast<DWORD **>(pBaseClass);
		m_pVTableOriginal = new DWORD[nFunctions];
		m_pVTable = *m_pBaseClass;

		m_nFunctions = nFunctions;

		memcpy(m_pVTableOriginal, m_pVTable, nFunctions * sizeof(void *));
	}
}

void CVTableHook::Init(void *pBaseClass)
{
	m_pBaseClass = static_cast<DWORD **>(pBaseClass);
	m_pVTable = *m_pBaseClass;
	m_nFunctions = 1;

	DWORD *pVTable = m_pVTable;
	DWORD dwModuleBase = *m_pVTable >> 20;

	while ((*++pVTable >> 20) == dwModuleBase)
		m_nFunctions++;

	m_pVTableOriginal = new DWORD[m_nFunctions];
	memcpy(m_pVTableOriginal, m_pVTable, m_nFunctions * sizeof(void *));
}

void CVTableHook::Remove()
{
	if (!m_nFunctions || !m_pVTableOriginal)
		return;

	DWORD dwProtection;

	VirtualProtect(m_pVTable, m_nFunctions * sizeof(void *), PAGE_EXECUTE_READWRITE, &dwProtection);

	memcpy(m_pVTable, m_pVTableOriginal, m_nFunctions * sizeof(void *));

	VirtualProtect(m_pVTable, m_nFunctions * sizeof(void *), dwProtection, &dwProtection);

	delete[] m_pVTableOriginal;

	m_pVTableOriginal = NULL;
}

void *CVTableHook::GetFunction(const int nIndex) const
{
	return reinterpret_cast<void *>(m_pVTableOriginal[nIndex]);
}

bool CVTableHook::HookFunction(void *pFunction, const int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nFunctions)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pVTable + nIndex, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwProtection);

	m_pVTable[nIndex] = (DWORD)pFunction;

	VirtualProtect(m_pVTable + nIndex, sizeof(void *), dwProtection, &dwProtection);

	return true;
}

bool CVTableHook::UnhookFunction(const int nIndex)
{
	if (nIndex < 0 || nIndex >= m_nFunctions)
		return false;

	DWORD dwProtection;

	VirtualProtect(m_pVTable + nIndex, sizeof(void *), PAGE_EXECUTE_READWRITE, &dwProtection);

	m_pVTable[nIndex] = m_pVTableOriginal[nIndex];

	VirtualProtect(m_pVTable + nIndex, sizeof(void *), dwProtection, &dwProtection);

	return true;
}

int CVTableHook::TotalFunctions() const
{
	return m_nFunctions;
}