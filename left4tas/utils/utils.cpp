// Utils

#include "utils.h"

bool g_bFailedInit = false;

//-----------------------------------------------------------------------------
// Iteratively get an interface from major to minor
//-----------------------------------------------------------------------------

void *GetInterface(CreateInterfaceFn interfaceFactory, const char *pszInterfaceVersion)
{
	void *pInterface = NULL;
	char *szInterfaceVersion = const_cast<char *>(pszInterfaceVersion);

	const size_t length = strlen(szInterfaceVersion), last_idx = length - 1, post_last_idx = length - 2;

	while (szInterfaceVersion[post_last_idx] != '0' || szInterfaceVersion[last_idx] != '0')
	{
		if (pInterface = interfaceFactory(szInterfaceVersion, NULL))
			return pInterface;

		if (szInterfaceVersion[last_idx] == '0')
		{
			szInterfaceVersion[last_idx] = '9';

			if (szInterfaceVersion[post_last_idx] != '0')
				--szInterfaceVersion[post_last_idx];
		}
		else
		{
			--szInterfaceVersion[last_idx];
		}
	}

	return pInterface;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

const wchar_t *CStringToWideCString(const char *pszString)
{
	const size_t length = strlen(pszString) + 1;
	wchar_t *wcString = new wchar_t[length];

	mbstowcs(wcString, pszString, length);

	return wcString;
}