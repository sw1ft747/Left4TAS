// Misc. Functions

#include "misc.h"

#include "utils.h"

#include "../offsets.h"
#include "../prop_offsets.h"
#include "../usermsg/usermessages.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern CGlobalVars *gpGlobals;
extern IServer *g_pServer;
extern IServerGameClients *g_pServerGameClients;
extern IVEngineServer *g_pEngineServer;
extern IPlayerInfoManager *g_pPlayerInfoManager;

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

static AcceptInputFn s_pfnAcceptInput = NULL;
static TeleportFn s_pfnTeleport = NULL;

UTIL_PlayerByIndexFn UTIL_PlayerByIndex = NULL; // CBasePlayer UTIL_PlayerByIndex(const int nIndex)
GoAwayFromKeyboardFn GoAwayFromKeyboard = NULL; // void GoAwayFromKeyboard(CTerrorPlayer *pPlayer)
TakeOverBotFn TakeOverBot = NULL; // bool TakeOverBot(CTerrorPlayer *pPlayer, bool bUnknown)
ForEachTerrorPlayer_FindCharacterFn ForEachTerrorPlayer_FindCharacter = NULL; // bool ForEachTerrorPlayer_FindCharacter(FindCharacter &characterEntry)

variant_t g_EmptyVariant;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

bool AcceptInput(CBaseEntity *pEntity, const char *pszInputName, CBaseEntity *pActivator, CBaseEntity *pCaller, variant_t Value, int outputID)
{
	if (!s_pfnAcceptInput)
		s_pfnAcceptInput = GetVTableFunction<AcceptInputFn>(pEntity, Offsets::Functions::CBaseEntity__AcceptInput);

	return s_pfnAcceptInput(pEntity, pszInputName, pActivator, pCaller, Value, outputID);
}

void Teleport(CBaseEntity *pEntity, const Vector *newPosition, const QAngle *newAngles, const Vector *newVelocity)
{
	if (!s_pfnTeleport)
		s_pfnTeleport = GetVTableFunction<TeleportFn>(pEntity, Offsets::Functions::CBaseEntity__Teleport);

	s_pfnTeleport(pEntity, newPosition, newAngles, newVelocity);
}

void SetName(int index, const char *szName)
{
	edict_t *pEdict = EntIndexToEdict(index);

	if (!IsEdictValid(pEdict))
		return;

	void *pBaseClient = g_pServer->GetClient(index - 1);

	// Pointer to CGameClient
	void *pGameClient = (void *)((DWORD)pBaseClient - sizeof(void *));

	// void CBaseClient::SetName( const char *name );
	GetVTableFunction<void(__thiscall *)(void *, const char *)>(pGameClient, Offsets::Functions::CBaseClient__SetName)(pGameClient, szName);

	g_pServerGameClients->ClientSettingsChanged(pEdict);
}

int CreateFakeClientTeam(const int nTeam)
{
	if (nTeam < 1 || nTeam > 3)
	{
		Warning("[CreateFakeClientTeam] Invalid team number\n");
		return -1;
	}

	edict_t *pEdict = g_pEngineServer->CreateFakeClient("");

	if (!pEdict)
		return -1;

	int index = EdictToEntIndex(pEdict);

	char buffer[5];
	sprintf(buffer, "%d", index);

	SetName(index, buffer);

	IPlayerInfo *pPlayerInfo = g_pPlayerInfoManager->GetPlayerInfo(pEdict);
	pPlayerInfo->ChangeTeam(2);

	return index;
}

int CreateFakeClientNamedTeam(const char *pszName, const int nTeam)
{
	if (nTeam < 1 || nTeam > 3)
	{
		Warning("[CreateFakeClientNamedTeam] Invalid team number\n");
		return -1;
	}

	edict_t *pEdict = g_pEngineServer->CreateFakeClient(pszName);

	if (!pEdict)
		return -1;

	IPlayerInfo *pPlayerInfo = g_pPlayerInfoManager->GetPlayerInfo(pEdict);
	pPlayerInfo->ChangeTeam(nTeam);

	return EdictToEntIndex(pEdict);
}

void SayText(int client, const char *pszMessage)
{
	if (client < 1)
	{
		MessageBegin(MSG_BROADCAST, TYPE_SAYTEXT);
		WriteByte(0);
		WriteString(pszMessage);
		WriteByte(0);
		MessageEnd();
	}
	else
	{
		edict_t *pEdict = EntIndexToEdict(client);

		if (!IsEdictValid(pEdict))
			return;

		MessageBegin(MSG_UNICAST_RELIABLE, TYPE_SAYTEXT, pEdict);
		WriteByte(client);
		WriteString(pszMessage);
		WriteByte(0);
		MessageEnd();
	}
}

void ClientPrint(int client, int dest, const char *pszMessage)
{
	/* Print destinations:
		Notify = 1
		Console = 2
		Chat = 3
		Center of HUD = 4
	*/

	if (client < 1)
	{
		MessageBegin(MSG_BROADCAST, TYPE_TEXTMSG);
		WriteByte(dest);
		WriteString(pszMessage);
		WriteString("");
		WriteString("");
		WriteString("");
		WriteString("");
		MessageEnd();
	}
	else
	{
		edict_t *pEdict = EntIndexToEdict(client);

		if (!IsEdictValid(pEdict))
			return;

		MessageBegin(MSG_UNICAST_RELIABLE, TYPE_TEXTMSG, pEdict);
		WriteByte(dest);
		WriteString(pszMessage);
		WriteString("");
		WriteString("");
		WriteString("");
		WriteString("");
		MessageEnd();
	}
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(tas_setname, "Set client's name")
{
	if (args.ArgC() < 4)
	{
		Msg("Usage: tas_setname [mode: 0 - index, 1 - character] [index/character] [name]\nCharacters: 0 - Nick/Bill, 1 - Rochelle/Zoey, 2 - Coach/Louis, 3 - Ellis/Francis\n");
		return;
	}

	int mode = atol(args.Arg(1));

	if (mode)
	{
		struct FindCharacter findCharacter = { static_cast<SurvivorCharacterType>(atol(args.Arg(2))), NULL };
		ForEachTerrorPlayer_FindCharacter(findCharacter);

		if (findCharacter.player)
			SetName(EntIndexOfBaseEntity((CBaseEntity *)findCharacter.player), args.Arg(3));
	}
	else
	{
		SetName(atol(args.Arg(2)), args.Arg(3));
	}
}

CON_COMMAND(tas_fake, "Create a fake client")
{
	int nArgs = args.ArgC();

	if (nArgs == 1)
	{
		CreateFakeClientTeam(2);
	}
	else if (nArgs == 2)
	{
		CreateFakeClientNamedTeam(args.Arg(1), 2);
	}
	else if (nArgs == 3)
	{
		if (*args.Arg(1) != '\0')
			CreateFakeClientNamedTeam(args.Arg(1), atol(args.Arg(2)));
		else
			CreateFakeClientTeam(atol(args.Arg(2)));
	}
	else
	{
		Msg("Usage: tas_fake [optional: name] [optional: team]\n");
	}
}

CON_COMMAND(tas_idle, "Go to IDLE mode")
{
	if (!GoAwayFromKeyboard || !ForEachTerrorPlayer_FindCharacter || !UTIL_PlayerByIndex)
		return;

	if (args.ArgC() < 3)
	{
		Msg("Usage: tas_idle [mode: 0 - index, 1 - character] [index/character]\nCharacters: 0 - Nick/Bill, 1 - Rochelle/Zoey, 2 - Coach/Louis, 3 - Ellis/Francis\n");
		return;
	}

	int mode = atol(args.Arg(1));

	if (mode)
	{
		struct FindCharacter findCharacter = { static_cast<SurvivorCharacterType>(atol(args.Arg(2))), NULL };
		ForEachTerrorPlayer_FindCharacter(findCharacter);

		if (findCharacter.player)
			GoAwayFromKeyboard(findCharacter.player);
	}
	else
	{
		CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(atol(args.Arg(2)));

		if (pPlayer)
			GoAwayFromKeyboard(pPlayer);
	}
}

CON_COMMAND(tas_take, "Take over a bot")
{
	if (!TakeOverBot || !ForEachTerrorPlayer_FindCharacter || !UTIL_PlayerByIndex)
		return;

	if (args.ArgC() < 3)
	{
		Msg("Usage: tas_take [mode: 0 - index, 1 - character] [index/character]\nCharacters: 0 - Nick/Bill, 1 - Rochelle/Zoey, 2 - Coach/Louis, 3 - Ellis/Francis\n");
		return;
	}

	int mode = atol(args.Arg(1));

	if (mode)
	{
		struct FindCharacter findCharacter = { static_cast<SurvivorCharacterType>(atol(args.Arg(2))), NULL };
		ForEachTerrorPlayer_FindCharacter(findCharacter);
		
		if (findCharacter.player)
		{
			int nHumanSpectatorIndex = *reinterpret_cast<int *>(GetOffset(findCharacter.player, NetPropOffsets::m_humanSpectatorEntIndex));

			if (nHumanSpectatorIndex > 0)
			{
				CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(nHumanSpectatorIndex);

				if (pPlayer)
					TakeOverBot(pPlayer, true);
			}
		}
	}
	else
	{
		CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(atol(args.Arg(2)));

		if (pPlayer)
			TakeOverBot(pPlayer, true);
	}
}