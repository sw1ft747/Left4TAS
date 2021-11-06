// C++
// Engine Module

#include "../cvars.h"
#include "../offsets.h"
#include "../patterns.h"
#include "../tools/timer.h"

#include "signature_scanner.h"
#include "trampoline_hook.h"
#include "vtable_hook.h"
#include "utils.h"

#include "vgui.h"
#include "client.h"
#include "server.h"
#include "engine.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern IEngineVGui *g_pEngineVGui;
extern IServer *g_pServer;
extern ICvar *g_pCVar;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static bool bForcePause = false;

bool g_bLevelChange = false;

void *g_pGetBaseLocalClient = NULL;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(SetPaused_Hook);

TRAMPOLINE_HOOK(Host_NewGame_Hook);
TRAMPOLINE_HOOK(Host_Changelevel_Hook);

VTABLE_HOOK(IEngineVGuiInternal_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

SetPausedFn SetPaused_Original = NULL;

Host_NewGameFn Host_NewGame_Original = NULL;
Host_ChangelevelFn Host_Changelevel_Original = NULL;

PaintFn Paint_Original = NULL;

Cbuf_AddTextFn Cbuf_AddText = NULL;
Cbuf_ExecuteFn Cbuf_Execute = NULL;

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
	}
	else
	{
		if (!bForcePause && prevent_unpause.GetBool())
			return;
	}

	bForcePause = false;
	SetPaused_Original(thisptr, paused);
}

bool Host_NewGame_Hooked(char *mapName, bool loadGame, bool bBackgroundLevel, bool bSplitScreenConnect, const char *pszOldMap, const char *pszLandmark)
{
	g_bLevelChange = false;
	g_bInTransition = false;

	g_Timer.Reset();

	return Host_NewGame_Original(mapName, loadGame, bBackgroundLevel, bSplitScreenConnect, pszOldMap, pszLandmark);
}

void Host_Changelevel_Hooked(bool loadfromsavedgame, const char *mapname, const char *start)
{
	g_bLevelChange = true;

	g_Timer.Reset();

	Host_Changelevel_Original(loadfromsavedgame, mapname, start);
}

void __fastcall Paint_Hooked(void *thisptr, int edx, PaintMode_t mode)
{
	if (g_bInSplitScreen)
		Paint_Original(thisptr, mode);

	if (IsVGUIModuleInit() && mode == PAINT_UIPANELS)
		DrawHUD();

	if (!g_bInSplitScreen)
		Paint_Original(thisptr, mode);
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

	void *pCbuf_AddText = FIND_PATTERN(L"engine.dll", Patterns::Engine::Cbuf_AddText);

	if (!pCbuf_AddText)
	{
		FailedInit("Cbuf_AddText");
		return false;
	}

	void *pCbuf_Execute = FIND_PATTERN(L"engine.dll", Patterns::Engine::Cbuf_Execute);

	if (!pCbuf_Execute)
	{
		FailedInit("Cbuf_Execute");
		return false;
	}

	g_pGetBaseLocalClient = FIND_PATTERN(L"engine.dll", Patterns::Engine::GetBaseLocalClient);

	if (!g_pGetBaseLocalClient)
	{
		FailedInit("GetBaseLocalClient");
		return false;
	}

	Cbuf_AddText = (Cbuf_AddTextFn)pCbuf_AddText;
	Cbuf_Execute = (Cbuf_ExecuteFn)pCbuf_Execute;

	HOOK_FUNCTION(SetPaused_Hook, GetVTableFunction(g_pServer, Offsets::Functions::IServer__SetPaused), SetPaused_Hooked, SetPaused_Original, SetPausedFn);
	HOOK_FUNCTION(Host_NewGame_Hook, pHost_NewGame, Host_NewGame_Hooked, Host_NewGame_Original, Host_NewGameFn);
	HOOK_FUNCTION(Host_Changelevel_Hook, pHost_Changelevel, Host_Changelevel_Hooked, Host_Changelevel_Original, Host_ChangelevelFn);

	HOOK_VTABLE(IEngineVGuiInternal_Hook, g_pEngineVGui, Offsets::Functions::IEngineVGuiInternal__Paint + 1);

	HOOK_VTABLE_FUNC(IEngineVGuiInternal_Hook, Paint_Hooked, Offsets::Functions::IEngineVGuiInternal__Paint, Paint_Original, PaintFn);

	__INITIALIZED__ = true;
	return true;
}

void ReleaseEngineModule()
{
	if (!__INITIALIZED__)
		return;

	UNHOOK_FUNCTION(SetPaused_Hook);
	UNHOOK_FUNCTION(Host_NewGame_Hook);
	UNHOOK_FUNCTION(Host_Changelevel_Hook);

	UNHOOK_VTABLE_FUNC(IEngineVGuiInternal_Hook, Offsets::Functions::IEngineVGuiInternal__Paint);

	REMOVE_VTABLE_HOOK(IEngineVGuiInternal_Hook);
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