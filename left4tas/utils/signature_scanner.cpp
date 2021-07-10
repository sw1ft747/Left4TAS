// C++
// Signature Scanner

#pragma once

#include "signature_scanner.h"

static const wchar_t *s_wcModuleName = NULL;
static MODULEINFO s_mInfo = { 0 };

static inline bool RetrieveModuleInfo(const wchar_t *wcModuleName)
{
	if (s_wcModuleName)
	{
		if (wcscmp(wcModuleName, s_wcModuleName))
		{
		GET_MODINFO:
			HMODULE hModule = GetModuleHandle(wcModuleName);

			if (!hModule)
				return true;

			GetModuleInformation(GetCurrentProcess(), hModule, &s_mInfo, sizeof(MODULEINFO));
		}
	}
	else
	{
		goto GET_MODINFO;
	}

	return false;
}

void *FindPatternWithOffset(const wchar_t *wcModuleName, BYTE byteSignature[], const char szMask[], const DWORD dwOffset)
{
	if (RetrieveModuleInfo(wcModuleName))
		return NULL;

	s_wcModuleName = wcModuleName;

	size_t i, nMaskLength = strlen(szMask);

	BYTE *pModuleBase = (PBYTE)s_mInfo.lpBaseOfDll + dwOffset;
	BYTE *pModuleEnd = pModuleBase + s_mInfo.SizeOfImage - (DWORD)nMaskLength;

	if ((DWORD)pModuleBase > (DWORD)pModuleEnd)
		return NULL;

	while (pModuleBase < pModuleEnd)
	{
		for (i = 0; i < nMaskLength; i++)
		{
			if (szMask[i] != '?' && byteSignature[i] != pModuleBase[i])
				break;
		}

		if (i == nMaskLength)
			return (void *)pModuleBase;

		pModuleBase++;
	}

	return NULL;
}

void *FindPatternWithOffset(const wchar_t *wcModuleName, BYTE byteSignature[], const DWORD dwLength, const DWORD dwOffset, const BYTE byteIgnore = 0x2A)
{
	if (RetrieveModuleInfo(wcModuleName))
		return NULL;

	s_wcModuleName = wcModuleName;

	size_t i, nSigLength = static_cast<size_t>(dwLength);

	BYTE *pModuleBase = (PBYTE)((DWORD)s_mInfo.lpBaseOfDll + dwOffset);
	BYTE *pModuleEnd = pModuleBase + s_mInfo.SizeOfImage - (DWORD)nSigLength;

	if ((DWORD)pModuleBase > (DWORD)pModuleEnd)
		return NULL;

	while (pModuleBase < pModuleEnd)
	{
		for (i = 0; i < nSigLength; i++)
		{
			if (byteSignature[i] != byteIgnore && byteSignature[i] != pModuleBase[i])
				break;
		}

		if (i == nSigLength)
			return (void *)pModuleBase;

		pModuleBase++;
	}

	return NULL;
}

void *LookupForStringWithOffset(const wchar_t *wcModuleName, const char *szString, DWORD dwOffset)
{
	if (RetrieveModuleInfo(wcModuleName))
		return NULL;

	s_wcModuleName = wcModuleName;

	size_t i, nLength = strlen(szString);

	BYTE *pModuleBase = (PBYTE)((DWORD)s_mInfo.lpBaseOfDll + dwOffset);
	BYTE *pModuleEnd = pModuleBase + s_mInfo.SizeOfImage - (DWORD)nLength;

	if ((DWORD)pModuleBase > (DWORD)pModuleEnd)
		return NULL;

	while (pModuleBase < pModuleEnd)
	{
		for (i = 0; i < nLength; i++)
		{
			if (szString[i] != pModuleBase[i])
				break;
		}

		if (i == nLength)
			return (void *)pModuleBase;

		pModuleBase++;
	}

	return NULL;
}

void *FindAddressWithOffset(const wchar_t *wcModuleName, void *pAddress, DWORD dwOffset)
{
	if (RetrieveModuleInfo(wcModuleName))
		return NULL;

	s_wcModuleName = wcModuleName;

	DWORD dwAddress = (DWORD)pAddress;

	BYTE *pModuleBase = (PBYTE)((DWORD)s_mInfo.lpBaseOfDll + dwOffset);
	BYTE *pModuleEnd = pModuleBase + s_mInfo.SizeOfImage - (DWORD)4;

	if ((DWORD)pModuleBase > (DWORD)pModuleEnd)
		return NULL;

	while (pModuleBase < pModuleEnd)
	{
		if (*(PDWORD)pModuleBase == dwAddress)
			return (void *)pModuleBase;

		pModuleBase++;
	}

	return NULL;
}