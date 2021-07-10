// C++
// Engine Module

#include "../cvars.h"
#include "../offsets.h"
#include "../patterns.h"
#include "../tools/timer.h"

#include "signature_scanner.h"
#include "trampoline_hook.h"
#include "utils.h"

#include "engine.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern IServer *g_pServer;
extern ICvar *g_pCVar;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static bool bForcePause = false;

bool g_bGamePaused = false;
bool g_bLevelChange = false;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(SetPaused_Hook);

TRAMPOLINE_HOOK(Host_NewGame_Hook);
TRAMPOLINE_HOOK(Host_Changelevel_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

SetPausedFn SetPaused_Original = NULL;

Host_NewGameFn Host_NewGame_Original = NULL;
Host_ChangelevelFn Host_Changelevel_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __fastcall SetPaused_Hooked(void *thisptr, int edx, bool paused)
{
	if (paused)
	{
		if (!bForcePause && prevent_pause.GetBool())
			return;

		g_Timer.OnPreciseTimeCorrupted();

		g_bGamePaused = true;
	}
	else
	{
		if (!bForcePause && prevent_unpause.GetBool())
			return;

		g_bGamePaused = false;
	}

	bForcePause = false;
	SetPaused_Original(thisptr, paused);
}

bool Host_NewGame_Hooked(char *mapName, bool loadGame, bool bBackgroundLevel, bool bSplitScreenConnect, const char *pszOldMap, const char *pszLandmark)
{
	g_bLevelChange = false;

	return Host_NewGame_Original(mapName, loadGame, bBackgroundLevel, bSplitScreenConnect, pszOldMap, pszLandmark);
}

void Host_Changelevel_Hooked(bool loadfromsavedgame, const char *mapname, const char *start)
{
	g_bLevelChange = true;

	Host_Changelevel_Original(loadfromsavedgame, mapname, start);
}

//-----------------------------------------------------------------------------
// Init/release engine module
//-----------------------------------------------------------------------------

bool IsEngineModuleInit()
{
	return __INITIALIZED__;
}

bool InitEngineModule()
{
	void *pHost_NewGame = FIND_PATTERN(L"engine.dll", Patterns::Engine::Host_NewGame);

	if (!pHost_NewGame)
	{
		FailedInit("Host_NewGame");
		return false;
	}

	void *pHost_Changelevel = FIND_PATTERN(L"engine.dll", Patterns::Engine::Host_Changelevel);

	if (!pHost_Changelevel)
	{
		FailedInit("Host_Changelevel");
		return false;
	}

	INIT_HOOK(SetPaused_Hook, GetVTableFunction(g_pServer, Offsets::Functions::IServer__SetPaused), SetPaused_Hooked);
	INIT_HOOK(Host_NewGame_Hook, pHost_NewGame, Host_NewGame_Hooked);
	INIT_HOOK(Host_Changelevel_Hook, pHost_Changelevel, Host_Changelevel_Hooked);

	HOOK_FUNC(SetPaused_Hook, SetPaused_Original, SetPausedFn);
	HOOK_FUNC(Host_NewGame_Hook, Host_NewGame_Original, Host_NewGameFn);
	HOOK_FUNC(Host_Changelevel_Hook, Host_Changelevel_Original, Host_ChangelevelFn);

	__INITIALIZED__ = true;
	return true;
}

void ReleaseEngineModule()
{
	if (!__INITIALIZED__)
		return;

	UNHOOK_FUNC(SetPaused_Hook);
	UNHOOK_FUNC(Host_NewGame_Hook);
	UNHOOK_FUNC(Host_Changelevel_Hook);

	REMOVE_HOOK(SetPaused_Hook);
	REMOVE_HOOK(Host_NewGame_Hook);
	REMOVE_HOOK(Host_Changelevel_Hook);
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(force_pause, "Pause the game")
{
	static ConVar *sv_pausable = NULL;

	if (!sv_pausable)
		sv_pausable = g_pCVar->FindVar("sv_pausable");

	bForcePause = true;

	sv_pausable->SetValue(1);
	g_pServer->SetPaused(true);
}

CON_COMMAND(force_unpause, "Unpause the game")
{
	bForcePause = true;
	g_pServer->SetPaused(false);
}

ConVar prevent_pause("prevent_pause", "1", FCVAR_RELEASE, "Prevent pausing except force_pause");
ConVar prevent_unpause("prevent_unpause", "1", FCVAR_RELEASE, "Prevent unpausing except force_unpause");