// VSTDLib Module

#include "vstdlib.h"

#include "utils.h"
#include "signature_scanner.h"

#include "trampoline_hook.h"

#include <stdio.h>
#include <string>

#define SEED_FILE_VERSION 1
#define SEED_FILE_UNIFORM_RANDOM_SIZE (34 * sizeof(DWORD))

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern std::string g_sSeedsDirectory;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CVSTDLib g_VSTDLib;

static char s_szSeedBuffer[512];

static void *pInstallUniformRandomStream = NULL;
static DWORD *s_UniformStream = NULL;

//-----------------------------------------------------------------------------
// VSTDLib module implementations
//-----------------------------------------------------------------------------

const char *CVSTDLib::GetSeedFilePath(const char *pszFilename) const
{
	static char s_szSeedBuffer[512];
	int result = sprintf_s(s_szSeedBuffer, sizeof(s_szSeedBuffer), "%s%s.bin", g_sSeedsDirectory.c_str(), pszFilename);

	if (result == -1)
	{
		Warning("CVSTDLib::GetSeedFilePath: buffer overflow\n");
		return NULL;
	}

	return s_szSeedBuffer;
}

void CVSTDLib::SaveSeed(const char *pszFilename) const
{
	if (!s_UniformStream)
		return;

	FILE *file = fopen(GetSeedFilePath(pszFilename), "wb");

	if (file)
	{
		char file_version = SEED_FILE_VERSION;
		DWORD *pUniformStream = (DWORD *)((DWORD)s_UniformStream + sizeof(void *)); // Don't save vtable

		fwrite(&file_version, 1, sizeof(char), file);

		// Save class members: int m_idum; int m_iy; int m_iv[32];
		fwrite(pUniformStream, 1, SEED_FILE_UNIFORM_RANDOM_SIZE, file);

		Msg("Seed has been saved to file %s.bin", pszFilename);
		fclose(file);
	}
	else
	{
		Warning("CVSTDLib::SaveSeed: failed to save seed\n");
	}
}

void CVSTDLib::SetSeed(const char *pszFilename)
{
	if (!s_UniformStream)
		return;

	FILE *file = fopen(GetSeedFilePath(pszFilename), "rb");

	if (file)
	{
		char file_version = SEED_FILE_VERSION;
		DWORD uniformRandomStream[34] = { 0 };

		fread(&file_version, 1, sizeof(char), file);

		if (file_version == SEED_FILE_VERSION)
		{
			fread(uniformRandomStream, 1, SEED_FILE_UNIFORM_RANDOM_SIZE, file);

			DWORD dwProtection;
			DWORD *pUniformStream = (DWORD *)((DWORD)s_UniformStream + sizeof(void *));

			VirtualProtect(pUniformStream, SEED_FILE_UNIFORM_RANDOM_SIZE, PAGE_EXECUTE_READWRITE, &dwProtection);
			memcpy(pUniformStream, uniformRandomStream, SEED_FILE_UNIFORM_RANDOM_SIZE);
			VirtualProtect(pUniformStream, SEED_FILE_UNIFORM_RANDOM_SIZE, dwProtection, &dwProtection);

			Msg("Seed has been set from file %s.bin", pszFilename);
		}

		fclose(file);
	}
	else
	{
		Warning("CVSTDLib::SetSeed: failed to set seed\n");
	}
}

CVSTDLib::CVSTDLib() : m_bInitialized(false)
{
}

bool CVSTDLib::IsInitialized() const
{
	return m_bInitialized;
}

bool CVSTDLib::Init()
{
	pInstallUniformRandomStream = FIND_PATTERN(L"vstdlib.dll", Patterns::VSTDLib::InstallUniformRandomStream);

	if (!pInstallUniformRandomStream)
	{
		FailedInit("InstallUniformRandomStream");
		return false;
	}

	s_UniformStream = *reinterpret_cast<DWORD **>(GetOffset(pInstallUniformRandomStream, Offsets::Variables::s_UniformStream));

	m_bInitialized = true;
	return true;
}

bool CVSTDLib::Release()
{
	if (!m_bInitialized)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(save_seed, "Save structure of singleton class UniformRandomStream")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: save_seed [filename]\n");
		return;
	}

	g_VSTDLib.SaveSeed(args.Arg(1));
}

CON_COMMAND(set_seed, "Set members of singleton class UniformRandomStream")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: set_seed [filename]\n");
		return;
	}

	g_VSTDLib.SetSeed(args.Arg(1));
}