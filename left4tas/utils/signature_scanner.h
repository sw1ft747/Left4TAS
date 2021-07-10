// C++
// Signature Scanner

#pragma once

#include <Windows.h>
#include <psapi.h>

#include "patterns_base.h"

// Pattern struct
#define FIND_PATTERN(module_name, name) FindPattern(module_name, name##_sig, name##_len);
#define FIND_PATTERN_IGNORE_BYTE(module_name, name, ignoreByte) FindPattern(module_name, name##_sig, name##_len, ignoreByte);

static inline bool RetrieveModuleInfo(const wchar_t *wcModuleName);

void *FindPatternWithOffset(const wchar_t *wcModuleName, BYTE byteSignature[], const char szMask[], const DWORD dwOffset);

void *FindPatternWithOffset(const wchar_t *wcModuleName, BYTE byteSignature[], const DWORD dwLength, const DWORD dwOffset, const BYTE byteIgnore);

void *LookupForStringWithOffset(const wchar_t *wcModuleName, const char *szString, DWORD dwOffset);

void *FindAddressWithOffset(const wchar_t *wcModuleName, void *pAddress, DWORD dwOffset);

inline void *FindPatternWithMask(const wchar_t *wcModuleName, BYTE byteSignature[], const char szMask[])
{
	return FindPatternWithOffset(wcModuleName, byteSignature, szMask, 0);
}

inline void *FindPattern(const wchar_t *wcModuleName, BYTE byteSignature[], const DWORD dwLength, const BYTE byteIgnore = 0x2A)
{
	return FindPatternWithOffset(wcModuleName, byteSignature, dwLength, 0, byteIgnore);
}

inline void *LookupForString(const wchar_t *wcModuleName, const char *szString)
{
	return LookupForStringWithOffset(wcModuleName, szString, 0);
}

inline void *FindAddress(const wchar_t *wcModuleName, void *pAddress)
{
	return FindAddressWithOffset(wcModuleName, pAddress, 0);
}