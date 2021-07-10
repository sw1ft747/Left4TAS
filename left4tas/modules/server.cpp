// C++
// Server Module

#include "vscript.h"
#include "../vscript/vscript_shared.h"

#include "../cvars.h"
#include "../patterns.h"
#include "../prop_offsets.h"
#include "../tools/timer.h"
#include "../tools/misc.h"
#include "../tools/netpropmanager.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "utils.h"
#include "usercmd.h"

#include "server.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum TimerStartType
{
	RoundRestart = 0,
	MapLoaded,
	IntroFinished,
	Custom
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern CGlobalVars *gpGlobals;
extern IVEngineClient *g_pEngineClient;
extern IVEngineServer *g_pEngineServer;
extern IServerTools *g_pServerTools;
extern IServer *g_pServer;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static bool bSegmentFinished = false;
static CBaseEntity *s_pTriggerFinale = NULL;

bool is_c5m5 = false;
bool is_c13m4 = false;

ConVar *host_timescale = NULL;
void OnTimeScaleChange(IConVar *var, const char *pOldValue, float flOldValue);

CGlobalEntityList *gEntList = NULL;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(PlayerRunCommand_Hook);

TRAMPOLINE_HOOK(TeamStartTouchIsValid_Hook);

TRAMPOLINE_HOOK(RestartRound_Hook);
TRAMPOLINE_HOOK(UnfreezeTeam_Hook);

TRAMPOLINE_HOOK(OnStartIntro_Hook);
TRAMPOLINE_HOOK(OnFinishIntro_Hook);
TRAMPOLINE_HOOK(OnBeginTransition_Hook);

TRAMPOLINE_HOOK(OnFinaleEscapeForceSurvivorPositions_Hook);
TRAMPOLINE_HOOK(DirectorOnFinaleEscapeForceSurvivorPositions_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

FindEntityByClassnameFn FindEntityByClassname = NULL;
FindEntityByClassnameFastFn FindEntityByClassnameFast = NULL;

PlayerRunCommandFn PlayerRunCommand_Original = NULL;
TeamStartTouchIsValidFn TeamStartTouchIsValid_Original = NULL;

RestartRoundFn RestartRound_Original = NULL;
UnfreezeTeamFn UnfreezeTeam_Original = NULL;

OnStartIntroFn OnStartIntro_Original = NULL;
OnFinishIntroFn OnFinishIntro_Original = NULL;
OnBeginTransitionFn OnBeginTransition_Original = NULL;

OnFinaleEscapeForceSurvivorPositionsFn OnFinaleEscapeForceSurvivorPositions_Original = NULL;
DirectorOnFinaleEscapeForceSurvivorPositionsFn DirectorOnFinaleEscapeForceSurvivorPositions_Original = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

inline CBaseEntity *CGlobalEntityList::FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName)
{
	return ::FindEntityByClassname(this, pStartEntity, szName);
}

inline CBaseEntity *CGlobalEntityList::FindEntityByClassnameFast(CBaseEntity *pStartEntity, const char *szName)
{
	return ::FindEntityByClassnameFast(this, pStartEntity, castable_string_t(szName));
}

// Don't remove '__declspec(noinline)' otherwise 'OnFinishIntro_Hooked' will crash your game, compiler adds
// the function prologue and epilogue when calling 'OnFinishIntro_Original', can't figure out how this is causing crash

void __declspec(noinline) OnTimerStart(TimerStartType type)
{
	if (tas_autoexec_configs.GetBool())
	{
		switch (type)
		{
		case RoundRestart:
			g_pEngineServer->ServerCommand("exec tas_round_start\n");
			break;

		case MapLoaded:
			g_pEngineServer->ServerCommand("exec tas_map_load\n");
			break;

		case IntroFinished:
			g_pEngineServer->ServerCommand("exec tas_intro_finish\n");
			break;
		}

		g_pEngineServer->ServerExecute();
	}

	if (tas_timer_callbacks.GetBool() && IsVScriptModuleInit())
	{
		HSCRIPT hOnTimerStartFunc = g_pScriptVM->LookupFunction("OnTimerStart");

		if (hOnTimerStartFunc)
		{
			if (g_pScriptVM->Call(hOnTimerStartFunc, NULL, true, NULL, static_cast<int>(type)) == SCRIPT_ERROR)
				Warning("Failed to call 'OnTimerStart' function\n");
		}
	}
}

void OnTimerStop(bool bCustom = false)
{
	if (tas_autoexec_configs.GetBool() && !bCustom)
	{
		g_pEngineServer->ServerCommand("exec tas_round_end\n");
		g_pEngineServer->ServerExecute();
	}

	if (tas_timer_callbacks.GetBool() && IsVScriptModuleInit())
	{
		HSCRIPT hOnTimerEndFunc = g_pScriptVM->LookupFunction("OnTimerEnd");

		if (hOnTimerEndFunc)
		{
			if (g_pScriptVM->Call(hOnTimerEndFunc, NULL, true, NULL) == SCRIPT_ERROR)
				Warning("Failed to call 'OnTimerEnd' function\n");
		}
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __fastcall PlayerRunCommand_Hooked(CBasePlayer *pPlayer, int edx, CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	PlayerRunCommand_Original(pPlayer, pCmd, pMoveHelper);
}

bool __fastcall TeamStartTouchIsValid_Hooked(void *thisptr, int edx, void *pTrigger)
{
	bool result = TeamStartTouchIsValid_Original(thisptr, pTrigger);
	
	if (result && (is_c5m5 || is_c13m4))
	{
		/** Explanation:
		 * Entity 'trigger_finale' is used to teleport survivors in the safe room during outro's
		 * During outro a main entity sends input 'OnFinaleEscapeForceSurvivorPositions' to teleport survivors
		 * That input also throws game event 'finale_vehicle_leaving'
		 *
		 * The Parish and Cold Stream finales don't spawn this entity until you activate the radio
		 * This is why I decided to spawn entity and remove it when my hook function handles call of 'CTriggerFinale::OnFinaleEscapeForceSurvivorPositions'
		*/

		CBaseEntity *pTriggerFinale = gEntList->FindEntityByClassnameFast(NULL, "trigger_finale");

		if (!pTriggerFinale)
		{
			s_pTriggerFinale = (CBaseEntity *)g_pServerTools->CreateEntityByName("trigger_finale");

			g_pServerTools->SetKeyValue(s_pTriggerFinale, "rendermode", 6);
			g_pServerTools->SetKeyValue(s_pTriggerFinale, "origin", Vector(0.0f, 0.0f, -10000000.0f));
			g_pServerTools->SetKeyValue(s_pTriggerFinale, "targetname", "finale");
			g_pServerTools->SetKeyValue(s_pTriggerFinale, "model", is_c13m4 ?
										"models/props_mill/freightelevatorbutton01.mdl" :
										"models/props_unique/generator_switch_01_outline.mdl");

			g_pServerTools->DispatchSpawn(s_pTriggerFinale);
		}
		else
		{
			s_pTriggerFinale = NULL;
		}
	}

	return result;
}

uintptr_t __fastcall RestartRound_Hooked(void *thisptr, int edx)
{
	if (tas_autorun_vscripts.GetBool() && IsVScriptModuleInit())
		VScriptRunScript("left4tas");

	auto result = RestartRound_Original(thisptr);
	extern bool g_bLevelChange;

	// Start timer
	if (!g_bLevelChange)
	{
		g_Timer.Reset();

		if (timer_auto.GetBool())
			g_Timer.Start();

		g_Timer.OnPreciseTimeCorrupted();
		OnTimerStart(RoundRestart);
	}

	s_pTriggerFinale = NULL;

	return result;
}

// Called when load to the map
uintptr_t __fastcall UnfreezeTeam_Hooked(void *thisptr, int edx)
{
	auto result = UnfreezeTeam_Original(thisptr);

	// Start timer
	if (timer_auto.GetBool())
	{
		g_Timer.Reset();
		g_Timer.Start();
	}

	g_Timer.OnPreciseTimeCorrupted();
	OnTimerStart(MapLoaded);

	return result;
}

void __fastcall OnStartIntro_Hooked(void *thisptr, int edx)
{
	// Reset timer
	if (timer_auto.GetBool())
		g_Timer.Reset();

	OnStartIntro_Original(thisptr);
}

void __fastcall OnFinishIntro_Hooked(void *thisptr, int edx)
{
	// Start timer
	if (timer_auto.GetBool())
	{
		g_Timer.Reset();
		g_Timer.Start();
	}

	OnTimerStart(IntroFinished);

	OnFinishIntro_Original(thisptr);
}

void __fastcall OnBeginTransition_Hooked(void *thisptr, int edx, bool bScenarioRestart)
{
	// Stop timer
	if (timer_auto.GetBool())
	{
		g_Timer.Stop();
		g_Timer.PrintTicks();
		g_Timer.PrintTime();
	}

	OnTimerStop();

	OnBeginTransition_Original(thisptr, bScenarioRestart);
}

void __fastcall OnFinaleEscapeForceSurvivorPositions_Hooked(void *thisptr, int edx, inputdata_t &inputData)
{
	OnFinaleEscapeForceSurvivorPositions_Original(thisptr, inputData);

	// Stop timer
	if (timer_auto.GetBool() && !bSegmentFinished)
	{
		g_Timer.Stop();
		g_Timer.PrintTicks();
		g_Timer.PrintTime();

		// c7m3's finale is spamming 'OnFinaleEscapeForceSurvivorPositions' input
		bSegmentFinished = true;
	}

	if (s_pTriggerFinale)
	{
		AcceptInput(s_pTriggerFinale, "Kill", s_pTriggerFinale, s_pTriggerFinale, g_EmptyVariant, 0);
		s_pTriggerFinale = NULL;
	}

	OnTimerStop();
}

void __fastcall DirectorOnFinaleEscapeForceSurvivorPositions_Hooked(void *thisptr, int edx, CUtlVector<CTerrorPlayer *, CUtlMemory<CTerrorPlayer *, int>> &players)
{
	if (s_pTriggerFinale)
		return;

	DirectorOnFinaleEscapeForceSurvivorPositions_Original(thisptr, players);
}

//-----------------------------------------------------------------------------
// Init/release server-side module
//-----------------------------------------------------------------------------

bool IsServerModuleInit()
{
	return __INITIALIZED__;
}

bool InitServerModule()
{
	if (!__AcceptInput)
		return false;

	// It works but so far it's useless
	//void *pPlayerRunCommand = FIND_PATTERN(L"server.dll", Patterns::Server::CBasePlayer__PlayerRunCommand);

	//if (!pPlayerRunCommand)
	//{
	//	FailedInit("CBasePlayer::PlayerRunCommand");
	//	return false;
	//}

	void *pRestartRound = FIND_PATTERN(L"server.dll", Patterns::Server::CTerrorGameRules__RestartRound);

	if (!pRestartRound)
	{
		FailedInit("CTerrorGameRules::RestartRound");
		return false;
	}

	void *pTeamStartTouchIsValid = FIND_PATTERN(L"server.dll", Patterns::Server::CDirectorChallengeMode__TeamStartTouchIsValid);

	if (!pTeamStartTouchIsValid)
	{
		FailedInit("CDirectorChallengeMode::TeamStartTouchIsValid");
		return false;
	}
	
	void *pUnfreezeTeam = FIND_PATTERN(L"server.dll", Patterns::Server::CDirectorSessionManager__UnfreezeTeam);

	if (!pUnfreezeTeam)
	{
		FailedInit("CDirectorSessionManager::UnfreezeTeam");
		return false;
	}
	
	void *pOnStartIntro = FIND_PATTERN(L"server.dll", Patterns::Server::CDirector__OnStartIntro);

	if (!pOnStartIntro)
	{
		FailedInit("CDirector::OnStartIntro");
		return false;
	}
	
	void *pOnFinishIntro = FIND_PATTERN(L"server.dll", Patterns::Server::CDirector__OnFinishIntro);

	if (!pOnFinishIntro)
	{
		FailedInit("CDirector::OnFinishIntro");
		return false;
	}
	
	void *pOnBeginTransition = FIND_PATTERN(L"server.dll", Patterns::Server::CDirector__OnBeginTransition);

	if (!pOnBeginTransition)
	{
		FailedInit("CDirector::OnBeginTransition");
		return false;
	}
	
	void *pOnFinaleEscapeForceSurvivorPositions2 = FIND_PATTERN(L"server.dll", Patterns::Server::CDirector__OnFinaleEscapeForceSurvivorPositions);

	if (!pOnFinaleEscapeForceSurvivorPositions2)
	{
		FailedInit("CDirector::OnFinaleEscapeForceSurvivorPositions");
		return false;
	}

	void *pOnFinaleEscapeForceSurvivorPositions = FIND_PATTERN(L"server.dll", Patterns::Server::CFinaleTrigger__OnFinaleEscapeForceSurvivorPositions);

	if (!pOnFinaleEscapeForceSurvivorPositions)
	{
		FailedInit("CFinaleTrigger::OnFinaleEscapeForceSurvivorPositions");
		return false;
	}
	
	void *pFindEntityByClassname = FIND_PATTERN(L"server.dll", Patterns::Server::CGlobalEntityList__FindEntityByClassname);

	if (!pFindEntityByClassname)
	{
		FailedInit("CGlobalEntityList::FindEntityByClassname");
		return false;
	}
	
	void *pFindEntityByClassnameFast = FIND_PATTERN(L"server.dll", Patterns::Server::CGlobalEntityList__FindEntityByClassnameFast);

	if (!pFindEntityByClassnameFast)
	{
		FailedInit("CGlobalEntityList::FindEntityByClassnameFast");
		return false;
	}

	/*
	void *func_hostage_rescue = LookupForString(L"server.dll", "func_h");
	BYTE *pCheckMapConditions = (BYTE *)GetOffset(FindAddress(L"server.dll", func_hostage_rescue), 4);

	// mov ecx, offset gEntList
	while (*pCheckMapConditions != 0xB9)
		++pCheckMapConditions;

	gEntList = *reinterpret_cast<CGlobalEntityList **>(++pCheckMapConditions);
	*/

	// gEntList
	BYTE *pOnStartIntro_gEntList = (BYTE *)GetOffset(pOnStartIntro, 16);

	while (*pOnStartIntro_gEntList != 0xB9)
		++pOnStartIntro_gEntList;
	
	gEntList = *reinterpret_cast<CGlobalEntityList **>(++pOnStartIntro_gEntList);
	FindEntityByClassname = (FindEntityByClassnameFn)pFindEntityByClassname;
	FindEntityByClassnameFast = (FindEntityByClassnameFastFn)pFindEntityByClassnameFast;

	// host_timescale
	(host_timescale = g_pCVar->FindVar("host_timescale"))->InstallChangeCallback(OnTimeScaleChange);

	//INIT_HOOK(PlayerRunCommand_Hook, pPlayerRunCommand, PlayerRunCommand_Hooked);
	INIT_HOOK(TeamStartTouchIsValid_Hook, pTeamStartTouchIsValid, TeamStartTouchIsValid_Hooked);
	INIT_HOOK(RestartRound_Hook, pRestartRound, RestartRound_Hooked);
	INIT_HOOK(UnfreezeTeam_Hook, pUnfreezeTeam, UnfreezeTeam_Hooked);
	INIT_HOOK(OnStartIntro_Hook, pOnStartIntro, OnStartIntro_Hooked);
	INIT_HOOK(OnFinishIntro_Hook, pOnFinishIntro, OnFinishIntro_Hooked);
	INIT_HOOK(OnBeginTransition_Hook, pOnBeginTransition, OnBeginTransition_Hooked);
	INIT_HOOK(OnFinaleEscapeForceSurvivorPositions_Hook, pOnFinaleEscapeForceSurvivorPositions, OnFinaleEscapeForceSurvivorPositions_Hooked);
	INIT_HOOK(DirectorOnFinaleEscapeForceSurvivorPositions_Hook, pOnFinaleEscapeForceSurvivorPositions2, DirectorOnFinaleEscapeForceSurvivorPositions_Hooked);

	//HOOK_FUNC(PlayerRunCommand_Hook, PlayerRunCommand_Original, PlayerRunCommandFn);
	HOOK_FUNC(TeamStartTouchIsValid_Hook, TeamStartTouchIsValid_Original, TeamStartTouchIsValidFn);
	HOOK_FUNC(RestartRound_Hook, RestartRound_Original, RestartRoundFn);
	HOOK_FUNC(UnfreezeTeam_Hook, UnfreezeTeam_Original, UnfreezeTeamFn);
	HOOK_FUNC(OnStartIntro_Hook, OnStartIntro_Original, OnStartIntroFn);
	HOOK_FUNC(OnFinishIntro_Hook, OnFinishIntro_Original, OnFinishIntroFn);
	HOOK_FUNC(OnBeginTransition_Hook, OnBeginTransition_Original, OnBeginTransitionFn);
	HOOK_FUNC(OnFinaleEscapeForceSurvivorPositions_Hook, OnFinaleEscapeForceSurvivorPositions_Original, OnFinaleEscapeForceSurvivorPositionsFn);
	HOOK_FUNC(DirectorOnFinaleEscapeForceSurvivorPositions_Hook, DirectorOnFinaleEscapeForceSurvivorPositions_Original, DirectorOnFinaleEscapeForceSurvivorPositionsFn);

	__INITIALIZED__ = true;
	return true;
}

void ReleaseServerModule()
{
	if (!__INITIALIZED__)
		return;

	// Reset class member: FnChangeCallback_t m_fnChangeCallback
	*reinterpret_cast<DWORD *>(GetOffset(host_timescale, 0x44)) = NULL;

	//UNHOOK_FUNC(PlayerRunCommand_Hook);
	UNHOOK_FUNC(TeamStartTouchIsValid_Hook);
	UNHOOK_FUNC(RestartRound_Hook);
	UNHOOK_FUNC(UnfreezeTeam_Hook);
	UNHOOK_FUNC(OnStartIntro_Hook);
	UNHOOK_FUNC(OnFinishIntro_Hook);
	UNHOOK_FUNC(OnBeginTransition_Hook);
	UNHOOK_FUNC(OnFinaleEscapeForceSurvivorPositions_Hook);
	UNHOOK_FUNC(DirectorOnFinaleEscapeForceSurvivorPositions_Hook);

	//REMOVE_HOOK(PlayerRunCommand_Hook);
	REMOVE_HOOK(TeamStartTouchIsValid_Hook);
	REMOVE_HOOK(RestartRound_Hook);
	REMOVE_HOOK(UnfreezeTeam_Hook);
	REMOVE_HOOK(OnStartIntro_Hook);
	REMOVE_HOOK(OnFinishIntro_Hook);
	REMOVE_HOOK(OnBeginTransition_Hook);
	REMOVE_HOOK(OnFinaleEscapeForceSurvivorPositions_Hook);
	REMOVE_HOOK(DirectorOnFinaleEscapeForceSurvivorPositions_Hook);
}

//-----------------------------------------------------------------------------
// Console variables/commands
//-----------------------------------------------------------------------------

CON_COMMAND(tas_tp, "Teleport to beginning position, optional: snap eye angles")
{
	if (args.ArgC() < 4)
	{
		Msg("Usage: tas_tp [x] [y] [z] [optional: pitch] [optional: yaw]\n");
		return;
	}

	edict_t *pEdict = EntIndexToEdict(g_pEngineClient->GetLocalPlayer());

	if (IsEdictValid(pEdict))
	{
		float X = static_cast<float>(atof(args.Arg(1)));
		float Y = static_cast<float>(atof(args.Arg(2)));
		float Z = static_cast<float>(atof(args.Arg(3)));

		QAngle *pViewAngles = NULL;

		Vector vecOrigin = { X, Y, Z };
		Vector vecVelocity = { 0.0f, 0.0f, 0.0f };

		CBaseEntity *pEntity = EdictToBaseEntity(pEdict);

		if (args.ArgC() >= 6)
		{
			float flPitch = static_cast<float>(atof(args.Arg(4)));
			float flYaw = static_cast<float>(atof(args.Arg(5)));

			QAngle viewangles = { flPitch, flYaw, 0.0f };
			pViewAngles = &viewangles;
		}

		Teleport(pEntity, &vecOrigin, pViewAngles, &vecVelocity);
	}
}

CON_COMMAND(tas_setcvar, "Set a value of the given ConVar")
{
	if (args.ArgC() != 3)
	{
		Msg("Usage: tas_setcvar [cvar_name] [value]\n");
		return;
	}

	ConVar *pCvar = g_pCVar->FindVar(args.Arg(1));

	if (!pCvar)
	{
		Warning("tas_setcvar: no such ConVar");
		return;
	}

	pCvar->SetValue(args.Arg(2));
}

CON_COMMAND(timer_set_segment_time, "Set the time from previous segment")
{
	if (args.ArgC() != 3)
	{
		Msg("Usage: timer_set_segment_time [in-game time] [precise time]\n");
		return;
	}

	float flSegmentTime = static_cast<float>(atof(args.Arg(1)));
	float flSegmentPreciseTime = static_cast<float>(atof(args.Arg(2)));

	if (!g_Timer.SetSegmentTime(flSegmentTime, flSegmentPreciseTime))
		Warning("timer_set_segment_time: the given value cannot be lower than zero!");
}

CON_COMMAND(timer_start, "Start the timer")
{
	g_Timer.Start();

	if (g_pServer->IsPaused())
		g_Timer.OnPreciseTimeCorrupted();

	OnTimerStart(Custom);
}

CON_COMMAND(timer_stop, "Stop the timer")
{
	g_Timer.Stop();
	OnTimerStop(true);
}

CON_COMMAND(timer_reset, "Reset the timer")
{
	g_Timer.Reset();
}

CON_COMMAND(timer_print_time, "Print the current/total time")
{
	g_Timer.PrintTime();
}

CON_COMMAND(timer_print_ticks, "Print the current/total ticks")
{
	g_Timer.PrintTicks();
}

ConVar tas_autoexec_configs("tas_autoexec_configs", "1", FCVAR_RELEASE, "Autoexec the following files:\n- tas_round_start.cfg\n- tas_map_load.cfg\n- tas_intro_finish.cfg\n- tas_round_end.cfg");
ConVar tas_autorun_vscripts("tas_autorun_vscripts", "1", FCVAR_RELEASE, "Autorun left4tas.nut file before round will start");
ConVar tas_timer_callbacks("tas_timer_callbacks", "1", FCVAR_RELEASE, "Call VScripts callbacks OnTimerStart and OnTimerEnd");

ConVar timer_auto("timer_auto", "0", FCVAR_RELEASE, "Auto start/end the timer");