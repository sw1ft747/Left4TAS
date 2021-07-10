// C++
// Patcher

#pragma once

#include <Windows.h>
#include <string.h>

#define PATCHER(name) CPatcher name;

#define INIT_PATCHER(name, address, patchedBytes, length) name.Init(address, patchedBytes, length);
#define REMOVE_PATCHER(name) name.Remove();

#define PATCH_MEMORY(name) name.Patch();
#define UNPATCH_MEMORY(name) name.Unpatch();

class CPatcher
{
public:
	CPatcher();

	void Init(void *pAddress, void *pPatchedBytes, const int nPatchLength);

	void Remove();

	bool Patch();

	bool Unpatch();

private:
	BYTE *m_pAddress;

	BYTE *m_pOriginalBytes;
	BYTE *m_pPatchedBytes;

	int m_nPatchLength;
};