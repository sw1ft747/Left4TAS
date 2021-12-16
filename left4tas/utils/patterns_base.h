// Patterns

#pragma once

#include <Windows.h>

#define PATTERN_LENGTH(pattern, name) DWORD name = pattern.length
#define PATTERN_SIGNATURE(pattern, name) BYTE *name = pattern.signature

#define INIT_PATTERN(name) extern DWORD (name##_len); extern BYTE *(name##_sig)

#define PATTERN(name, signature) struct Pattern<GetPatternLength(signature)> name(signature, 0x2A); PATTERN_LENGTH(name, name##_len); PATTERN_SIGNATURE(name, name##_sig)
#define PATTERN_IGNORE_BYTE(name, signature, ignorebyte) struct Pattern<GetPatternLength(signature)> name(signature, ignorebyte); PATTERN_LENGTH(name, name##_len); PATTERN_SIGNATURE(name, name##_sig)

#define REPLACE_PATTERN(pattern1, pattern2) pattern1##_len = pattern2##_len; pattern1##_sig = pattern2##_sig

static constexpr DWORD GetPatternLength(const char *szPattern)
{
	DWORD nCount = 0;

	while (*szPattern)
	{
		char symbol = *szPattern;

		if (symbol != ' ')
		{
			++nCount;

			if (symbol != '?')
				++szPattern;
		}

		++szPattern;
	}

	return nCount;
}

template<DWORD bytesCount>
struct Pattern
{
	constexpr Pattern(const char *szPattern, BYTE byteIgnore) : signature(), length(bytesCount)
	{
		DWORD nCount = 0;

		while (*szPattern)
		{
			char symbol = *szPattern;

			if (symbol != ' ')
			{
				if (symbol != '?')
				{
					char byte[3] = { 0 };

					byte[0] = szPattern[0];
					byte[1] = szPattern[1];

					signature[nCount] = static_cast<BYTE>(strtol(byte, NULL, 16));

					++szPattern;
				}
				else
				{
					signature[nCount] = byteIgnore;
				}

				++nCount;
			}

			++szPattern;
		}
	}

	DWORD length;
	BYTE signature[bytesCount];
};