// C++
// VSTDLib Module

#include "vstdlib.h"

#include "utils.h"
#include "signature_scanner.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;

void *pInstallUniformRandomStream = NULL;
DWORD *s_UniformStream = NULL;

//-----------------------------------------------------------------------------
// Init/release VSTDLib modle
//-----------------------------------------------------------------------------

bool IsVSTDLibModuleInit()
{
	return __INITIALIZED__;
}

bool InitVSTDLibModule()
{
	pInstallUniformRandomStream = FIND_PATTERN(L"vstdlib.dll", Patterns::VSTDLib::InstallUniformRandomStream);

	if (!pInstallUniformRandomStream)
	{
		FailedInit("InstallUniformRandomStream");
		return false;
	}

	s_UniformStream = *reinterpret_cast<DWORD **>(GetOffset(pInstallUniformRandomStream, Offsets::Variables::s_UniformStream));

	__INITIALIZED__ = true;
	return true;
}

void ReleaseVSTDLibModule()
{
	if (!__INITIALIZED__)
		return;
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(get_seed, "Print structure of class UniformRandomStream")
{
	extern DWORD *s_UniformStream;

	if (!s_UniformStream)
		return;

	char buffer[512];
	int shift = 0;

	DWORD *pUniformStream = (DWORD *)((DWORD)s_UniformStream + sizeof(void *)); // skipping vtable

	// Print members: int m_idum; int m_iy; int m_iv[32];

	for (int i = 0; i < 34; i++)
	{
		shift += sprintf(buffer + shift, (i != 33) ? "%X " : "%X", pUniformStream[i]);
	}

	Msg("Seed: %s\n", buffer);
}