// C++
// Auto Jump

#pragma once

#include "patcher.h"

class CAutoJump
{
public:
	CAutoJump();

	void Init();

	void Release();

	void Enable();

	void Disable();

	bool IsInitialized() const;

	bool IsEnabled() const;

private:
	CPatcher m_CheckJumpButtonServerPatch;
	CPatcher m_CheckJumpButtonClientPatch;

	bool m_bInitialized;
	bool m_bEnabled;
};

extern CAutoJump g_AutoJump;

extern void *g_pCheckJumpButtonServer;
extern void *g_pCheckJumpButtonClient;