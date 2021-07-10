// C++
// Misc. Functions

#include "misc.h"

#include "utils.h"
#include "../offsets.h"
#include "../usermsg/usermessages.h"

//-----------------------------------------------------------------------------
// Interfaces
//-----------------------------------------------------------------------------

extern CGlobalVars *gpGlobals;
extern IServer *g_pServer;
extern IServerGameClients *g_pServerGameClients;
extern IVEngineServer *g_pEngineServer;
extern IPlayerInfoManager *g_pPlayerInfoManager;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

variant_t g_EmptyVariant;

//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

GoAwayFromKeyboardFn __GoAwayFromKeyboard = NULL;
TakeOverBotFn __TakeOverBot = NULL;
UTIL_PlayerByIndexFn __UTIL_PlayerByIndex = NULL;
AcceptInputFn __AcceptInput = NULL;
ForEachTerrorPlayer_FindCharacterFn __ForEachTerrorPlayer_FindCharacter = NULL;

void SetName(int index, const char *szName)
{
	edict_t *pEdict = EntIndexToEdict(index);

	if (!IsEdictValid(pEdict))
		return;

	void *pBaseClient = g_pServer->GetClient(index - 1);

	// Pointer to CGameClient
	void *pGameClient = (void *)((DWORD)pBaseClient - sizeof(void *));

	// void CBaseClient::SetName( const char *name );
	GetVTableFunction<void (__thiscall *)(void *, const char *)>(pGameClient, Offsets::Functions::CBaseClient__SetName)(pGameClient, szName);

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
	if (args.ArgC() < 3)
	{
		Msg("Usage: tas_setname [index] [name]\n");
		return;
	}

	SetName(atol(args.Arg(1)), args.Arg(2));
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

CON_COMMAND(tas_idle, "")
{
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

CON_COMMAND(tas_take, "")
{
	if (args.ArgC() < 3)
	{
		Msg("Usage: tas_idle [mode: 0 - index, 1 - character] [index/character]\nCharacters: 0 - Nick/Bill, 1 - Coach/Louis, 2 - Rochelle/Zoey, 3 - Ellis/Francis\n");
		return;
	}

	int mode = atol(args.Arg(1));

	if (mode)
	{
		struct FindCharacter findCharacter = { static_cast<SurvivorCharacterType>(atol(args.Arg(2))), NULL };
		ForEachTerrorPlayer_FindCharacter(findCharacter);

		if (findCharacter.player)
			TakeOverBot(findCharacter.player, true);
	}
	else
	{
		CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(atol(args.Arg(2)));

		if (pPlayer)
			TakeOverBot(pPlayer, true);
	}
}