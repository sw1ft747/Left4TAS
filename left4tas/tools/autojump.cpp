// C++
// Auto Jump

#include "autojump.h"

#include "signature_scanner.h"
#include "utils.h"
#include "../patterns.h"
#include "../sdk.h"

CAutoJump g_AutoJump;

CAutoJump::CAutoJump() : m_bInitialized(false), m_bEnabled(false)
{
}

void CAutoJump::Init()
{
	if (!g_pCheckJumpButtonServer)
		g_pCheckJumpButtonServer = FIND_PATTERN(L"server.dll", Patterns::Server::CGameMovement__CheckJumpButton);

	if (!g_pCheckJumpButtonClient)
		g_pCheckJumpButtonClient = FIND_PATTERN(L"client.dll", Patterns::Client::CGameMovement__CheckJumpButton);

	if (g_pCheckJumpButtonServer && g_pCheckJumpButtonClient)
	{
		const BYTE pPatchedBytes[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

		INIT_PATCHER(m_CheckJumpButtonServerPatch, g_pCheckJumpButtonServer, (void *)pPatchedBytes, 5);
		INIT_PATCHER(m_CheckJumpButtonClientPatch, g_pCheckJumpButtonClient, (void *)pPatchedBytes, 5);

		m_bInitialized = true;

		return;
	}

	FailedInit("CAutoJump::Init");
}

void CAutoJump::Release()
{
	if (IsInitialized())
	{
		Disable();

		REMOVE_PATCHER(m_CheckJumpButtonServerPatch);
		REMOVE_PATCHER(m_CheckJumpButtonClientPatch);

		m_bInitialized = false;
	}
}

void CAutoJump::Enable()
{
	if (IsEnabled())
		return;

	PATCH_MEMORY(m_CheckJumpButtonServerPatch);
	PATCH_MEMORY(m_CheckJumpButtonClientPatch);

	m_bEnabled = true;
}

void CAutoJump::Disable()
{
	if (!IsEnabled())
		return;

	UNPATCH_MEMORY(m_CheckJumpButtonServerPatch);
	UNPATCH_MEMORY(m_CheckJumpButtonClientPatch);

	m_bEnabled = false;
}

bool CAutoJump::IsInitialized() const
{
	return m_bInitialized;
}

bool CAutoJump::IsEnabled() const
{
	return m_bEnabled;
}