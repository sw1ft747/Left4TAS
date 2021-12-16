// Console Variables

#include "cvars.h"

#include "tools/timer.h"

#include "modules/client.h"
#include "game/cl_splitscreen.h"

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

	if (!strcmp(pszName, "tas_timescale"))
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
		g_pEngineServer->ServerCommand(cvar->GetBool() ?
									   "director_no_bosses 1;director_no_mobs 1;director_no_specials 1;z_common_limit 0;z_mega_mob_size 0;nb_delete_all infected\n" :
									   "director_no_bosses 0;director_no_mobs 0;director_no_specials 0;z_common_limit 30;z_mega_mob_size 50\n");
	}
	else
	{
		static ConVar *survivor_limit = NULL;

		if (!survivor_limit)
			survivor_limit = g_pCVar->FindVar("survivor_limit");

		switch (cvar->GetInt())
		{
		case 1:
			survivor_limit->SetValue(1);
			g_pEngineServer->ServerCommand("nb_delete_all survivor\n");
			break;

		case 2:
			survivor_limit->SetValue(2);
			g_pEngineServer->ServerCommand("nb_delete_all survivor\n");
			break;

		default:
			survivor_limit->SetValue(4);
			g_pEngineServer->ServerCommand("sb_add;sb_add;sb_add\n");
			break;
		}
	}

	g_pEngineServer->ServerExecute();
}

void OnSetAngle(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;

	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();
	const char *pszName = var->GetName();

	if (!strcmp(pszName, "tas_setpitch"))
		g_Client.GetInputAction(slot).m_bSetPitch = true;
	else
		g_Client.GetInputAction(slot).m_bSetYaw = true;
}

void OnTimeScaleChange(IConVar *var, const char *pOldValue, float flOldValue)
{
	CVAR_INIT;

	static ConVar *sv_player_stuck_tolerance = NULL;

	if (!sv_player_stuck_tolerance)
		sv_player_stuck_tolerance = g_pCVar->FindVar("sv_player_stuck_tolerance");

	// Fix teleport time. ToDo: is fair to change?
	sv_player_stuck_tolerance->SetValue(10.0f * (1.0f / flValue));

	if (flValue != 1.0f)
		g_Timer.OnPreciseTimeCorrupted();
}