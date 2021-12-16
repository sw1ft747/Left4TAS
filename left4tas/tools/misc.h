// Misc. Functions

#pragma once

#include "../sdk.h"
#include "../game/server/variant_t.h"
#include "../game/functors.h"

#include "utils.h"
#include "../offsets.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CBasePlayer;
class CTerrorPlayer;

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

typedef CBasePlayer *(*UTIL_PlayerByIndexFn)(const int nIndex);

typedef void (__thiscall *GoAwayFromKeyboardFn)(CTerrorPlayer *pPlayer);
typedef bool (__thiscall *TakeOverBotFn)(CTerrorPlayer *pPlayer, bool bUnknown);

typedef void (__thiscall *TeleportFn)(CBaseEntity *pEntity, const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity);
typedef bool (__thiscall *AcceptInputFn)(CBaseEntity *pEntity, const char *pszInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID);

typedef bool (*ForEachTerrorPlayer_FindCharacterFn)(FindCharacter &characterEntry);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern variant_t g_EmptyVariant;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

extern UTIL_PlayerByIndexFn UTIL_PlayerByIndex;
extern GoAwayFromKeyboardFn GoAwayFromKeyboard;
extern TakeOverBotFn TakeOverBot;
extern ForEachTerrorPlayer_FindCharacterFn ForEachTerrorPlayer_FindCharacter;

bool AcceptInput(CBaseEntity *pEntity, const char *pszInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID);

void Teleport(CBaseEntity *pEntity, const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity);

void SetName(int index, const char *szName);

int CreateFakeClientTeam(const int nTeam);

int CreateFakeClientNamedTeam(const char *pszName, const int nTeam);

void SayText(int client, const char *pszMessage);

void ClientPrint(int client, int dest, const char *pszMessage);

inline int CreateFakeClient()
{
	return CreateFakeClientTeam(2); // switch to Survivors by default
}

inline int CreateFakeClientNamed(const char *pszName)
{
	return CreateFakeClientNamedTeam(pszName, 2);
}