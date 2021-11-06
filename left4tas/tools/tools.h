// C++
// Include all tools

#pragma once

#include "misc.h"
#include "recvpropmanager.h"
#include "netpropmanager.h"

#include "../prop_offsets.h"

extern IBaseClientDLL *g_pClient;
extern IServerGameDLL *g_pServerGameDLL;

inline void InitTools()
{
	RecvProps.Init(GetVTableFunction<ClientClass *(*)()>(g_pClient, Offsets::Functions::IBaseClientDLL__GetAllClasses)());
	NetProps.Init(g_pServerGameDLL->GetAllServerClasses());

	__UTIL_PlayerByIndex = (UTIL_PlayerByIndexFn)FIND_PATTERN(L"server.dll", Patterns::Server::UTIL_PlayerByIndex);

	if (!__UTIL_PlayerByIndex)
		FailedInit("UTIL_PlayerByIndex");

	__GoAwayFromKeyboard = (GoAwayFromKeyboardFn)FIND_PATTERN(L"server.dll", Patterns::Server::CTerrorPlayer__GoAwayFromKeyboard);

	if (!__GoAwayFromKeyboard)
		FailedInit("CTerrorPlayer::GoAwayFromKeyboard");

	__TakeOverBot = (TakeOverBotFn)FIND_PATTERN(L"server.dll", Patterns::Server::CTerrorPlayer__TakeOverBot);

	if (!__TakeOverBot)
		FailedInit("CTerrorPlayer::TakeOverBot");
	
	__ForEachTerrorPlayer_FindCharacter = (ForEachTerrorPlayer_FindCharacterFn)FIND_PATTERN(L"server.dll", Patterns::Server::ForEachTerrorPlayer_FindCharacter);

	if (!__ForEachTerrorPlayer_FindCharacter)
		FailedInit("ForEachTerrorPlayer<FindCharacter>");

	NetPropOffsets::m_humanSpectatorEntIndex = NetProps.GetPropOffset("SurvivorBot", "m_humanSpectatorEntIndex");

	// AcceptInput
	DWORD *pEmptyVariant = reinterpret_cast<DWORD *>(&g_EmptyVariant);

	*pEmptyVariant = NULL;
	pEmptyVariant[3] = INVALID_EHANDLE_INDEX;
	pEmptyVariant[4] = FIELD_VOID;
}

inline void ReleaseTools()
{
}