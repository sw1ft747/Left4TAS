// C++
// Console Variables

#include "cvars.h"
#include "tools/timer.h"
#include "tools/autojump.h"
#include "modules/client.h"

#define CVAR_INIT ConVar *cvar = dynamic_cast<ConVar *>(var); float flValue = cvar->GetFloat()
#define CVAR_IGNORE_NOT_CHANGED if (flValue == flOldValue) return

extern IServer *g_pServer;
extern IVEngineServer *g_pEngineServer;

// Callbacks
void OnConVarChange(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;
	CVAR_IGNORE_NOT_CHANGED;

	const char *pszName = var->GetName();

	if (!strcmp(pszName, "tas_autojump"))
	{
		if (cvar->GetBool())
			g_AutoJump.Enable();
		else
			g_AutoJump.Disable();
	}
	else if (!strcmp(pszName, "tas_timescale"))
	{
		if (flValue > 0.0f)
		{
			extern ConVar *host_timescale;
			host_timescale->SetValue(flValue);
		}
	}
}

void OnCategoryChange(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;

	const char *pszName = var->GetName();

	if (!strcmp(pszName, "category_no_director"))
	{
		g_pEngineServer->ServerCommand(cvar->GetBool() ? "director_stop;nb_delete_all infected\n" : "director_start\n");
	}
	//else if (!strcmp(pszName, "category_no_survivor_bots"))
	else
	{
		g_pEngineServer->ServerCommand(cvar->GetBool() ? "nb_delete_all survivor\n" : "sb_add;sb_add;sb_add\n");
	}

	g_pEngineServer->ServerExecute();
}

void OnSetAngle(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;

	const char *pszName = var->GetName();

	if (!strcmp(pszName, "tas_setpitch"))
		g_InputState.bSetPitch = true;
	else
		g_InputState.bSetYaw = true;
}

void OnTimeScaleChange(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;

	if (flValue != 1.0f)
		g_Timer.OnPreciseTimeCorrupted();
}

// ConVars
ConVar tas_autojump("tas_autojump", "1", FCVAR_RELEASE, "Player automatically re-jumps while holding jump button", true, 0.0f, true, 1.0f, OnConVarChange);
ConVar tas_timescale("tas_timescale", "1.0", FCVAR_CHEAT | FCVAR_RELEASE | FCVAR_REPLICATED, "Set server's timescale", OnConVarChange);

ConVar wait_frames_pause("wait_frames_pause", "0", FCVAR_RELEASE, "Pause execution of wait_frames commands");

ConVar category_no_director("category_no_director", "0", FCVAR_CHEAT | FCVAR_RELEASE, "No Director category", true, 0.0f, true, 1.0f, OnCategoryChange);
ConVar category_no_survivor_bots("category_no_survivor_bots", "0", FCVAR_CHEAT | FCVAR_RELEASE, "No Survivor Bots category", true, 0.0f, true, 1.0f, OnCategoryChange);

ConVar tas_setpitch("tas_setpitch", "0", FCVAR_RELEASE, "Set the Pitch angle.", OnSetAngle);

ConVar tas_setyaw("tas_setyaw", "0", FCVAR_RELEASE, "Set the Yaw angle.", OnSetAngle);

ConVar tas_setanglespeed("tas_setanglespeed", "360", FCVAR_RELEASE, "Speed of setting angles when using tas_setpitch/tas_setyaw.");