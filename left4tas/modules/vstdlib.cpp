// C++
// VSTDLib Module

#include "vstdlib.h"

#include "utils.h"
#include "signature_scanner.h"

#include "trampoline_hook.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static char s_szSeedBuffer[512];

void *pInstallUniformRandomStream = NULL;
DWORD *s_UniformStream = NULL;

#define VSTDLIB_TRACE 0

#if VSTDLIB_TRACE
FILE *tracing = NULL;

typedef void (*RandomSeedFn)(int);
typedef float (*RandomFloatFn)(float, float);
typedef float (*RandomFloatExpFn)(float, float, float);
typedef int (*RandomIntFn)(int, int);
typedef float (*RandomGaussianFloatFn)(float, float);
typedef void (*InstallUniformRandomStreamFn)(uintptr_t);

RandomSeedFn RandomSeed_Original = NULL;
RandomFloatFn RandomFloat_Original = NULL;
RandomFloatExpFn RandomFloatExp_Original = NULL;
RandomIntFn RandomInt_Original = NULL;
RandomGaussianFloatFn RandomGaussianFloat_Original = NULL;
InstallUniformRandomStreamFn InstallUniformRandomStream_Original = NULL;

TRAMPOLINE_HOOK(RandomSeed_Hook);
TRAMPOLINE_HOOK(RandomFloat_Hook);
TRAMPOLINE_HOOK(RandomFloatExp_Hook);
TRAMPOLINE_HOOK(RandomInt_Hook);
TRAMPOLINE_HOOK(RandomGaussianFloat_Hook);
TRAMPOLINE_HOOK(InstallUniformRandomStream_Hook);

void RandomSeed_Hooked(int iSeed)
{
	if (tracing)
		fprintf(tracing, "[RandomSeed] iSeed = %d\n", iSeed);

	RandomSeed_Original(iSeed);
}

float RandomFloat_Hooked(float flMinVal, float flMaxVal)
{
	float result = RandomFloat_Original(flMinVal, flMaxVal);

	if (tracing)
		fprintf(tracing, "[RandomFloat] Result = %f, flMinVal = %f, flMaxVal = %f\n", result, flMinVal, flMaxVal);

	return result;
}

float RandomFloatExp_Hooked(float flMinVal, float flMaxVal, float flExponent)
{
	float result = RandomFloatExp_Original(flMinVal, flMaxVal, flExponent);

	if (tracing)
		fprintf(tracing, "[RandomFloatExp] Result = %f, flMinVal = %f, flMaxVal = %f, flExponent = %f\n", result, flMinVal, flMaxVal, flExponent);

	return result;
}

int	RandomInt_Hooked(int iMinVal, int iMaxVal)
{
	int result = RandomInt_Original(iMinVal, iMaxVal);

	if (tracing)
		fprintf(tracing, "[RandomInt] Result = %d, iMinVal = %d, iMaxVal = %d\n", result, iMinVal, iMaxVal);

	return result;
}

float RandomGaussianFloat_Hooked(float flMean, float flStdDev)
{
	float result = RandomGaussianFloat_Original(flMean, flStdDev);

	if (tracing)
		fprintf(tracing, "[RandomGaussianFloat] Result = %f, flMean = %f, flStdDev = %f\n", result, flMean, flStdDev);

	return result;
}

void InstallUniformRandomStream_Hooked(uintptr_t pStream)
{
	if (tracing)
		fprintf(tracing, "[InstallUniformRandomStream] pStream = %X\n", pStream);

	InstallUniformRandomStream_Original(pStream);
}
#endif

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

#if VSTDLIB_TRACE
	DWORD dwVSTDLib = (DWORD)GetModuleHandle(L"vstdlib.dll");

	// Offsets for 2.1.5.0
	INIT_HOOK(RandomSeed_Hook, (void *)(dwVSTDLib + 0xDFE0), RandomSeed_Hooked);
	INIT_HOOK(RandomFloat_Hook, (void *)(dwVSTDLib + 0xE000), RandomFloat_Hooked);
	INIT_HOOK(RandomFloatExp_Hook, (void *)(dwVSTDLib + 0xE030), RandomFloatExp_Hooked);
	INIT_HOOK(RandomInt_Hook, (void *)(dwVSTDLib + 0xE060), RandomInt_Hooked);
	INIT_HOOK(RandomGaussianFloat_Hook, (void *)(dwVSTDLib + 0xE890), RandomGaussianFloat_Hooked);
	INIT_HOOK(InstallUniformRandomStream_Hook, (void *)(dwVSTDLib + 0xDFC0), InstallUniformRandomStream_Hooked);

	HOOK_FUNC(RandomSeed_Hook, RandomSeed_Original, RandomSeedFn);
	HOOK_FUNC(RandomFloat_Hook, RandomFloat_Original, RandomFloatFn);
	HOOK_FUNC(RandomFloatExp_Hook, RandomFloatExp_Original, RandomFloatExpFn);
	HOOK_FUNC(RandomInt_Hook, RandomInt_Original, RandomIntFn);
	HOOK_FUNC(RandomGaussianFloat_Hook, RandomGaussianFloat_Original, RandomGaussianFloatFn);
	HOOK_FUNC(InstallUniformRandomStream_Hook, InstallUniformRandomStream_Original, InstallUniformRandomStreamFn);
#endif

	__INITIALIZED__ = true;
	return true;
}

void ReleaseVSTDLibModule()
{
	if (!__INITIALIZED__)
		return;

#if VSTDLIB_TRACE
	UNHOOK_FUNC(RandomSeed_Hook);
	UNHOOK_FUNC(RandomFloat_Hook);
	UNHOOK_FUNC(RandomFloatExp_Hook);
	UNHOOK_FUNC(RandomInt_Hook);
	UNHOOK_FUNC(RandomGaussianFloat_Hook);
	UNHOOK_FUNC(InstallUniformRandomStream_Hook);

	REMOVE_HOOK(RandomSeed_Hook);
	REMOVE_HOOK(RandomFloat_Hook);
	REMOVE_HOOK(RandomFloatExp_Hook);
	REMOVE_HOOK(RandomInt_Hook);
	REMOVE_HOOK(RandomGaussianFloat_Hook);
	REMOVE_HOOK(InstallUniformRandomStream_Hook);
#endif
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

#if VSTDLIB_TRACE
CON_COMMAND(vstdlib_start_tracing, "")
{
	if (tracing)
		return;

	tracing = fopen("random.log", "w");
}

CON_COMMAND(vstdlib_stop_tracing, "")
{
	if (tracing)
	{
		fclose(tracing);
		tracing = NULL;
	}
}
#endif

CON_COMMAND(get_seed, "Print structure of class UniformRandomStream")
{
	if (!s_UniformStream)
		return;

	int shift = 0;

	DWORD *pUniformStream = (DWORD *)((DWORD)s_UniformStream + sizeof(void *)); // skipping vtable

	// Print members: int m_idum; int m_iy; int m_iv[32];

	for (int i = 0; i < 34; i++)
		shift += sprintf(s_szSeedBuffer + shift, (i != 33) ? "%X " : "%X", pUniformStream[i]);

	Msg("Seed: %s\n", s_szSeedBuffer);
}

CON_COMMAND(set_seed, "Change members of class UniformRandomStream")
{
	if (!s_UniformStream)
		return;

	DWORD uniformRandomStream[34];
	FILE *file = fopen("seed.txt", "r");

	if (file)
	{
		fgets(s_szSeedBuffer, ARRAYSIZE(s_szSeedBuffer), file);
		fclose(file);

		int it = 0;
		char *pszSeedMember = strtok(s_szSeedBuffer, " ");

		while (pszSeedMember != NULL)
		{
			uniformRandomStream[it] = strtol(pszSeedMember, NULL, 16);
			pszSeedMember = strtok(NULL, " ");

			++it;

			if (it >= ARRAYSIZE(uniformRandomStream))
				break;
		}

		if (it < ARRAYSIZE(uniformRandomStream))
		{
			Warning("set_seed: bad seed\n");
			return;
		}

		DWORD dwProtection;
		DWORD nLength = sizeof(DWORD) * ARRAYSIZE(uniformRandomStream);
		DWORD *pUniformStream = (DWORD *)((DWORD)s_UniformStream + sizeof(void *));

		VirtualProtect(pUniformStream, nLength, PAGE_EXECUTE_READWRITE, &dwProtection);

		memcpy(pUniformStream, uniformRandomStream, nLength);

		VirtualProtect(pUniformStream, nLength, dwProtection, &dwProtection);
	}
	else
	{
		Warning("set_seed: failed to open file seed.txt\n");
	}
}