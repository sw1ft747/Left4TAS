// Include/init all tools

#include "../prop_offsets.h"
#include "../patterns.h"

#include "signature_scanner.h"

#include "tools.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern IBaseClientDLL *g_pClient;
extern IServerGameDLL *g_pServerGameDLL;

//-----------------------------------------------------------------------------
// Initialize various tools
//-----------------------------------------------------------------------------

void InitTools()
{
	g_InputManager.Init();

	// Receive/network properties manager
	RecvProps.Init(GetVTableFunction<ClientClass *(*)()>(g_pClient, Offsets::Functions::IBaseClientDLL__GetAllClasses)());
	NetProps.Init(g_pServerGameDLL->GetAllServerClasses());

	// Misc. functions
	UTIL_PlayerByIndex = (UTIL_PlayerByIndexFn)FIND_PATTERN(L"server.dll", Patterns::Server::UTIL_PlayerByIndex);

	if (!UTIL_PlayerByIndex)
		FailedInit("UTIL_PlayerByIndex");

	GoAwayFromKeyboard = (GoAwayFromKeyboardFn)FIND_PATTERN(L"server.dll", Patterns::Server::CTerrorPlayer__GoAwayFromKeyboard);

	if (!GoAwayFromKeyboard)
		FailedInit("CTerrorPlayer::GoAwayFromKeyboard");

	TakeOverBot = (TakeOverBotFn)FIND_PATTERN(L"server.dll", Patterns::Server::CTerrorPlayer__TakeOverBot);

	if (!TakeOverBot)
		FailedInit("CTerrorPlayer::TakeOverBot");

	ForEachTerrorPlayer_FindCharacter = (ForEachTerrorPlayer_FindCharacterFn)FIND_PATTERN(L"server.dll", Patterns::Server::ForEachTerrorPlayer_FindCharacter);

	if (!ForEachTerrorPlayer_FindCharacter)
		FailedInit("ForEachTerrorPlayer<FindCharacter>");

	// Network properties
	NetPropOffsets::m_humanSpectatorEntIndex = NetProps.GetPropOffset("SurvivorBot", "m_humanSpectatorEntIndex");

	// AcceptInput
	DWORD *pEmptyVariant = reinterpret_cast<DWORD *>(&g_EmptyVariant);

	*pEmptyVariant = NULL;
	pEmptyVariant[3] = INVALID_EHANDLE_INDEX;
	pEmptyVariant[4] = FIELD_VOID;
}

void ReleaseTools()
{
}