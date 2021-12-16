// VSTDLib Module

#pragma once

#include "../offsets.h"
#include "../patterns.h"
#include "../sdk.h"

//-----------------------------------------------------------------------------
// VSTDLib Module
//-----------------------------------------------------------------------------

class CVSTDLib
{
public:
	CVSTDLib();

	bool Init();
	bool Release();

	bool IsInitialized() const;

public:
	void SaveSeed(const char *pszFilename) const;
	void SetSeed(const char *pszFilename);

private:
	const char *GetSeedFilePath(const char *pszFilename) const;

private:
	bool m_bInitialized;
};

extern CVSTDLib g_VSTDLib;