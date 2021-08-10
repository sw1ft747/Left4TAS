// C++
// Misc. Functions

#pragma once

#include "../sdk.h"
#include "../game/server/variant_t.h"
#include "../structs/functors.h"

#include "utils.h"
#include "../offsets.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CBasePlayer;
class CTerrorPlayer;

extern variant_t g_EmptyVariant;

//-----------------------------------------------------------------------------
// Typedefs
//-----------------------------------------------------------------------------

typedef CBasePlayer *(*UTIL_PlayerByIndexFn)(const int nIndex);
typedef void (__thiscall *GoAwayFromKeyboardFn)(CTerrorPlayer *pTerrorPlayer);
typedef bool (__thiscall *TakeOverBotFn)(CTerrorPlayer *pTerrorPlayer, bool bUnknown);

typedef void (__thiscall *TeleportFn)(void *, const Vector *, const QAngle *, const Vector *);
typedef bool (__thiscall *AcceptInputFn)(CBaseEntity *pEntity, const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID);

typedef bool (*ForEachTerrorPlayer_FindCharacterFn)(FindCharacter &);

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

extern GoAwayFromKeyboardFn __GoAwayFromKeyboard;
extern TakeOverBotFn __TakeOverBot;
extern UTIL_PlayerByIndexFn __UTIL_PlayerByIndex;
extern ForEachTerrorPlayer_FindCharacterFn __ForEachTerrorPlayer_FindCharacter;

void SetName(int index, const char *szName);

int CreateFakeClientTeam(const int nTeam);

int CreateFakeClientNamedTeam(const char *pszName, const int nTeam);

void SayText(int client, const char *pszMessage);

void ClientPrint(int client, int dest, const char *pszMessage);

inline int CreateFakeClient()
{
	return CreateFakeClientTeam(2);
}

inline int CreateFakeClientNamed(const char *pszName)
{
	return CreateFakeClientNamedTeam(pszName, 2);
}

inline CBasePlayer *UTIL_PlayerByIndex(const int nIndex)
{
	return __UTIL_PlayerByIndex(nIndex);
}

inline void GoAwayFromKeyboard(CTerrorPlayer *pTerrorPlayer)
{
	__GoAwayFromKeyboard(pTerrorPlayer);
}

inline bool TakeOverBot(CTerrorPlayer *pTerrorPlayer, bool bUnknown)
{
	return __TakeOverBot(pTerrorPlayer, bUnknown);
}

inline bool AcceptInput(CBaseEntity *pEntity, const char *szInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID)
{
	static AcceptInputFn __AcceptInput = NULL;

	if (!__AcceptInput)
		__AcceptInput = GetVTableFunction<AcceptInputFn>(pEntity, Offsets::Functions::CBaseEntity__AcceptInput);

	return __AcceptInput(pEntity, szInputName, pActivator, pCaller, Value, outputID);
}

inline void Teleport(CBaseEntity *pEntity, const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity)
{
	static TeleportFn __Teleport = NULL;

	if (!__Teleport)
		__Teleport = GetVTableFunction<TeleportFn>(pEntity, Offsets::Functions::CBaseEntity__Teleport);

	__Teleport(pEntity, newPosition, newAngles, newVelocity);
}

inline bool ForEachTerrorPlayer_FindCharacter(FindCharacter &findCharacter)
{
	return __ForEachTerrorPlayer_FindCharacter(findCharacter);
}