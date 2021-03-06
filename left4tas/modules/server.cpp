// Server Module

#include "vscript.h"
#include "../vscript/vscript_shared.h"

#include "../cvars.h"
#include "../patterns.h"
#include "../prop_offsets.h"

#include "../tools/timer.h"
#include "../tools/misc.h"
#include "../tools/netpropmanager.h"
#include "../tools/input_manager.h"

#include "../game/cl_splitscreen.h"
#include "../game/inputdata_t.h"

#include "game/shared/igamemovement.h"
#include "igameevents.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "utils.h"
#include "usercmd.h"

#include "../sdk.h"

#include "server.h"
#include "client.h"
#include "engine.h"

#define IS_PLAYER_INDEX_VALID(index) if (index >= 1 && index <= MAXCLIENTS)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef CBaseEntity *(__thiscall *FindEntityByClassnameFn)(void *, CBaseEntity *, const char *);
typedef CBaseEntity *(__thiscall *FindEntityByClassnameFastFn)(void *, CBaseEntity *, string_t);

typedef void (__thiscall *PlayerRunCommandFn)(CBasePlayer *, CUserCmd *, IMoveHelper *);
typedef bool (__thiscall *CheckJumpButtonServerFn)(void *);

typedef void (__thiscall *CDirectorSessionManager__FireGameEventFn)(void *, IGameEvent *);
typedef void (__cdecl *RestoreTransitionedEntitiesFn)();

typedef bool (__thiscall *TeamStartTouchIsValidFn)(void *, void *);
typedef void (__thiscall *RestartRoundFn)(void *);

typedef void (__thiscall *UnfreezeTeamFn)(void *);

typedef void (__thiscall *OnStartIntroFn)(void *);
typedef void (__thiscall *OnFinishIntroFn)(void *);
typedef void (__thiscall *OnBeginTransitionFn)(void *, bool);

typedef void (__thiscall *OnFinaleEscapeForceSurvivorPositionsFn)(void *, inputdata_t &);
typedef void (__thiscall *DirectorOnFinaleEscapeForceSurvivorPositionsFn)(void *, CUtlVector<CTerrorPlayer *, CUtlMemory<CTerrorPlayer *, int>> &);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern CGlobalVars *gpGlobals;
extern IVEngineClient *g_pEngineClient;
extern IVEngineServer *g_pEngineServer;
extern IServerTools *g_pServerTools;
extern IServer *g_pServer;

void OnTimeScaleChange(IConVar *var, const char *pOldValue, float flOldValue);
void OnConVarChange(IConVar *var, const char *pOldValue, float flOldValue);
void OnCategoryChange(IConVar *var, const char *pOldValue, float flOldValue);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CServer g_Server;

static CBaseEntity *s_pTriggerFinale = NULL;

CGlobalEntityList *gEntList = NULL;

bool is_c5m5 = false;
bool is_c13m4 = false;

//-----------------------------------------------------------------------------
// Native cvars
//-----------------------------------------------------------------------------

ConVar *host_timescale = NULL;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(PlayerRunCommand_Hook);
TRAMPOLINE_HOOK(CheckJumpButtonServer_Hook);

TRAMPOLINE_HOOK(CDirectorSessionManager__FireGameEvent_Hook);
TRAMPOLINE_HOOK(RestoreTransitionedEntities_Hook);

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

PlayerRunCommandFn PlayerRunCommand_Original = NULL;
CheckJumpButtonServerFn CheckJumpButtonServer_Original = NULL;

FindEntityByClassnameFn FindEntityByClassname = NULL;
FindEntityByClassnameFastFn FindEntityByClassnameFast = NULL;

CDirectorSessionManager__FireGameEventFn CDirectorSessionManager__FireGameEvent_Original = NULL;
RestoreTransitionedEntitiesFn RestoreTransitionedEntities_Original = NULL;

TeamStartTouchIsValidFn TeamStartTouchIsValid_Original = NULL;
RestartRoundFn RestartRound_Original = NULL;

UnfreezeTeamFn UnfreezeTeam_Original = NULL;

OnStartIntroFn OnStartIntro_Original = NULL;
OnFinishIntroFn OnFinishIntro_Original = NULL;
OnBeginTransitionFn OnBeginTransition_Original = NULL;

OnFinaleEscapeForceSurvivorPositionsFn OnFinaleEscapeForceSurvivorPositions_Original = NULL;
DirectorOnFinaleEscapeForceSurvivorPositionsFn DirectorOnFinaleEscapeForceSurvivorPositions_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __fastcall PlayerRunCommand_Hooked(CBasePlayer *pPlayer, int edx, CUserCmd *pCmd, IMoveHelper *pMoveHelper)
{
	int nPlayerIndex = EntIndexOfBaseEntity((CBaseEntity *)pPlayer);
	InputData *pInputData = g_InputManager.GetServerInputData(nPlayerIndex);

	if (pInputData->input)
	{
		if (pInputData->recording)
		{
			g_InputManager.SaveInput(pInputData, pCmd, false);
		}
		else
		{
			g_InputManager.ReadInput(pInputData, pCmd, pPlayer, false);
			Teleport((CBaseEntity *)pPlayer, NULL, &pCmd->viewangles, NULL);
		}

		++pInputData->frames;
	}

	PlayerRunCommand_Original(pPlayer, pCmd, pMoveHelper);
}

bool __fastcall CheckJumpButtonServer_Hooked(IGameMovement *thisptr)
{
	bool bJumped = g_Server.CheckJumpButton(thisptr);

	return bJumped;
}

void __fastcall CDirectorSessionManager__FireGameEvent_Hooked(void *thisptr, int edx, IGameEvent *event)
{
	CDirectorSessionManager__FireGameEvent_Original(thisptr, event);

	// First player fully spawned, we can start the timer (another way?)
	if (g_Server.IsTransitioning() && !strcmp("player_connect_full", event->GetName()))
	{
		g_Timer.Reset();

		// Start timer
		if (timer_auto.GetBool())
			g_Timer.Start();

		g_Timer.OnPreciseTimeCorrupted();
		g_Server.FireServerCallback(CB_TRANSITION_FINISHED);

		g_Engine.SetLevelChangeState(false);
		g_Server.SetTransitionState(false);
	}
}

void __cdecl RestoreTransitionedEntities_Hooked()
{
	RestoreTransitionedEntities_Original();

	// Last server's transition action (restoration of saved items) was completed
	// Purpose: create fake clients (maybe..) and let them control spawned survivor bots (probably game event 'player_connect_full' will fire for them, fix it)

	if (g_Server.IsTransitioning())
		g_Server.FireServerCallback(CB_SERVER_TRANSITION_FINISHED);
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

void __fastcall RestartRound_Hooked(void *thisptr)
{
	if (g_VScript.IsInitialized() && tas_autorun_vscripts.GetBool())
		VScriptRunScript("left4tas");

	RestartRound_Original(thisptr);

	// Start timer
	if (!g_Engine.IsLevelChanging())
	{
		g_Timer.Reset();

		if (timer_auto.GetBool())
			g_Timer.Start();

		g_Timer.OnPreciseTimeCorrupted();
		g_Server.FireServerCallback(CB_ROUND_RESTART);
	}

	s_pTriggerFinale = NULL;
}

// Called when load to the map
void __fastcall UnfreezeTeam_Hooked(void *thisptr)
{
	UnfreezeTeam_Original(thisptr);

	g_Timer.Reset();

	// Start timer
	if (timer_auto.GetBool())
		g_Timer.Start();

	g_Timer.OnPreciseTimeCorrupted();
	g_Server.FireServerCallback(CB_MAP_LOADED);
}

void __fastcall OnStartIntro_Hooked(void *thisptr)
{
	// Reset timer
	if (timer_auto.GetBool())
		g_Timer.Reset();

	OnStartIntro_Original(thisptr);
}

void __fastcall OnFinishIntro_Hooked(void *thisptr)
{
	// Start timer
	if (timer_auto.GetBool())
	{
		g_Timer.Reset();
		g_Timer.Start();
	}

	g_Server.FireServerCallback(CB_INTRO_FINISHED);

	OnFinishIntro_Original(thisptr);
}

void __fastcall OnBeginTransition_Hooked(void *thisptr, int edx, bool bScenarioRestart)
{
	g_Server.SetTransitionState(true);

	if (timer_auto.GetBool())
	{
		// Stop timer
		g_Timer.Stop();

		g_Timer.PrintTicks();
		g_Timer.PrintTime();

		g_Timer.SetSegmentTime(g_Timer.GetTime() + g_Timer.GetSegmentTime(), g_Timer.GetTime(true) + g_Timer.GetSegmentTime(true));
	}

	g_Server.FireServerCallback(CB_TIMER_STOPPED);

	OnBeginTransition_Original(thisptr, bScenarioRestart);
}

void __fastcall OnFinaleEscapeForceSurvivorPositions_Hooked(void *thisptr, int edx, inputdata_t &inputData)
{
	OnFinaleEscapeForceSurvivorPositions_Original(thisptr, inputData);

	// c7m3's finale is spamming 'OnFinaleEscapeForceSurvivorPositions' output
	if (!g_Server.IsSegmentFinished())
	{
		if (timer_auto.GetBool())
		{
			// Stop timer
			g_Timer.Stop();

			g_Timer.PrintTicks();
			g_Timer.PrintTime();
		}

		g_Server.FireServerCallback(CB_TIMER_STOPPED);
		g_Server.SetSegmentFinishedState(true);
	}

	if (s_pTriggerFinale)
	{
		AcceptInput(s_pTriggerFinale, "Kill", s_pTriggerFinale, s_pTriggerFinale, g_EmptyVariant, 0);
		s_pTriggerFinale = NULL;
	}
}

void __fastcall DirectorOnFinaleEscapeForceSurvivorPositions_Hooked(void *thisptr, int edx, CUtlVector<CTerrorPlayer *, CUtlMemory<CTerrorPlayer *, int>> &players)
{
	if (s_pTriggerFinale)
		return;

	DirectorOnFinaleEscapeForceSurvivorPositions_Original(thisptr, players);
}

//-----------------------------------------------------------------------------
// CGlobalEntityList implementations
//-----------------------------------------------------------------------------

inline CBaseEntity *CGlobalEntityList::FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName)
{
	return ::FindEntityByClassname(this, pStartEntity, szName);
}

inline CBaseEntity *CGlobalEntityList::FindEntityByClassnameFast(CBaseEntity *pStartEntity, const char *szName)
{
	return ::FindEntityByClassnameFast(this, pStartEntity, castable_string_t(szName));
}

//-----------------------------------------------------------------------------
// Server-side functions for Input Manager
//-----------------------------------------------------------------------------

void server__tas_im_record(const char *pszFilename, int nPlayerIndex)
{
	IS_PLAYER_INDEX_VALID(nPlayerIndex)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(nPlayerIndex);

		if (pPlayer)
		{
			float orientation[3][3];

			orientation[1][0] = *reinterpret_cast<float *>(GetOffset(pPlayer, NetPropOffsets::m_angEyeAnglesPitch));
			orientation[1][1] = *reinterpret_cast<float *>(GetOffset(pPlayer, NetPropOffsets::m_angEyeAnglesYaw));
			orientation[1][2] = 0.0f;

			*reinterpret_cast<Vector *>(orientation[0]) = *reinterpret_cast<Vector *>(GetOffset(pPlayer, NetPropOffsets::m_vecOrigin));
			*reinterpret_cast<Vector *>(orientation[2]) = *reinterpret_cast<Vector *>(GetOffset(pPlayer, NetPropOffsets::m_vecVelocity));

			g_InputManager.Record(pszFilename, g_InputManager.GetServerInputData(nPlayerIndex), orientation);
		}
	}
}

void server__tas_im_play(const char *pszFilename, int nPlayerIndex)
{
	IS_PLAYER_INDEX_VALID(nPlayerIndex)
	{
		CBasePlayer *pPlayer = UTIL_PlayerByIndex(nPlayerIndex);
		InputData *pInputData = g_InputManager.GetServerInputData(nPlayerIndex);

		if (pPlayer)
		{
			g_InputManager.Playback(pszFilename, pInputData);

			if (pInputData->active && UTIL_PlayerByIndex && tas_im_tp.GetBool())
			{
				Teleport((CBaseEntity *)pPlayer,
							(Vector *)pInputData->baseInfo.origin,
							(QAngle *)pInputData->baseInfo.viewangles,
							(Vector *)pInputData->baseInfo.velocity);
			}
		}
	}
}

void server__tas_im_split(int nPlayerIndex)
{
	IS_PLAYER_INDEX_VALID(nPlayerIndex)
		g_InputManager.Split(g_InputManager.GetServerInputData(nPlayerIndex));
}

void server__tas_im_stop(int nPlayerIndex)
{
	IS_PLAYER_INDEX_VALID(nPlayerIndex)
		g_InputManager.Stop(g_InputManager.GetServerInputData(nPlayerIndex));
}

//-----------------------------------------------------------------------------
// Server module implementations
//-----------------------------------------------------------------------------

bool CServer::CheckJumpButton(IGameMovement *pGameMovement)
{
	CMoveData *mv = reinterpret_cast<CMoveData *>(((uintptr_t *)pGameMovement)[2]);
	const int IN_JUMP = mv->m_nOldButtons & (1 << 1);

	if (tas_autojump.GetBool() && m_bAutoJump[EntIndexOfBaseEntity((CBaseEntity *)((uintptr_t *)pGameMovement)[1])])
		mv->m_nOldButtons &= ~IN_JUMP;

	bool bJumped = CheckJumpButtonServer_Original(pGameMovement);

	mv->m_nOldButtons |= IN_JUMP;
	return bJumped;
}

// Keep this method not inlined otherwise 'OnFinishIntro_Hooked' will crash your game
void __declspec(noinline) CServer::FireServerCallback(ServerCallbacks type, bool bTimerStoppedOutsidePlugin /* = false */)
{
	g_Server.SetSegmentFinishedState(false);

	if (tas_autoexec_configs.GetBool())
	{
		switch (type)
		{
		case CB_ROUND_RESTART:
			g_pEngineServer->ServerCommand("exec left4tas/tas_round_start\n");
			break;

		case CB_MAP_LOADED:
			g_pEngineServer->ServerCommand("exec left4tas/tas_map_load\n");
			break;

		case CB_INTRO_FINISHED:
			g_pEngineServer->ServerCommand("exec left4tas/tas_intro_finish\n");
			break;

		case CB_SERVER_TRANSITION_FINISHED:
			g_pEngineServer->ServerCommand("exec left4tas/tas_server_transition\n");
			break;

		case CB_TRANSITION_FINISHED:
			g_pEngineServer->ServerCommand("exec left4tas/tas_map_transition\n");
			break;

		case CB_TIMER_STOPPED:
			if (bTimerStoppedOutsidePlugin)
				goto EXECUTE_VSCRIPT_CALLBACK;

			g_pEngineServer->ServerCommand("exec left4tas/tas_segment_finish\n");
			break;
		}

		g_pEngineServer->ServerExecute();
	}

EXECUTE_VSCRIPT_CALLBACK:
	if (tas_vscript_callbacks.GetBool() && g_VScript.IsInitialized())
	{
		HSCRIPT hCallbacksFunc = g_pScriptVM->LookupFunction("Left4TAS_Callbacks");

		if (hCallbacksFunc)
		{
			// Game will when try to access to 'null' variable (for example, call a class method) in a script. Why?
			if (g_pScriptVM->Call(hCallbacksFunc, NULL, true, NULL, static_cast<int>(type)) == SCRIPT_ERROR)
				Warning("[L4TAS] Failed to call 'Left4TAS_Callbacks' function\n");
		}
	}
}

CServer::CServer() : m_bInitialized(false), m_bAutoJump()
{
	m_bInTransition = false;
	m_bSegmentFinished = false;
}

bool CServer::IsInitialized() const
{
	return m_bInitialized;
}

bool CServer::Init()
{
	memset(m_bAutoJump, true, sizeof(m_bAutoJump));

	void *pPlayerRunCommand = FIND_PATTERN(L"server.dll", Patterns::Server::CBasePlayer__PlayerRunCommand);

	if (!pPlayerRunCommand)
	{
		FailedInit("CBasePlayer::PlayerRunCommand");
		return false;
	}

	void *pCheckJumpButtonServer = FIND_PATTERN(L"server.dll", Patterns::Server::CGameMovement__CheckJumpButton);

	if (!pCheckJumpButtonServer)
	{
		FailedInit("CGameMovement::CheckJumpButton");
		return false;
	}

	void *pDirectorSessionManager__FireGameEvent = FIND_PATTERN(L"server.dll", Patterns::Server::CDirectorSessionManager__FireGameEvent);

	if (!pDirectorSessionManager__FireGameEvent)
	{
		FailedInit("CDirectorSessionManager::FireGameEvent");
		return false;
	}

	void *pRestoreTransitionedEntities = FIND_PATTERN(L"server.dll", Patterns::Server::RestoreTransitionedEntities);

	if (!pRestoreTransitionedEntities)
	{
		FailedInit("RestoreTransitionedEntities");
		return false;
	}

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

	// mov ecx, offset gEntList
	while (*++pOnStartIntro_gEntList != 0xB9) { }
	
	// Init global entity list
	gEntList = *reinterpret_cast<CGlobalEntityList **>(++pOnStartIntro_gEntList);
	FindEntityByClassname = (FindEntityByClassnameFn)pFindEntityByClassname;
	FindEntityByClassnameFast = (FindEntityByClassnameFastFn)pFindEntityByClassnameFast;

	// Set callback function for host_timescale cvar
	(host_timescale = g_pCVar->FindVar("host_timescale"))->InstallChangeCallback(OnTimeScaleChange);

	// Get offsets of network properties
	NetPropOffsets::m_vecOrigin = NetProps.GetPropOffset("CBaseEntity", "m_vecOrigin");
	NetPropOffsets::m_vecVelocity = NetProps.GetPropOffset("CBasePlayer", "m_vecVelocity[0]");
	NetPropOffsets::m_angEyeAnglesPitch = NetProps.GetPropOffset("CCSPlayer", "m_angEyeAngles[0]");
	NetPropOffsets::m_angEyeAnglesYaw = NetProps.GetPropOffset("CCSPlayer", "m_angEyeAngles[1]");
	NetPropOffsets::m_hMyWeapons = NetProps.GetPropOffset("CBasePlayer", "m_hMyWeapons");

	// Trampoline hook
	HOOK_FUNCTION(PlayerRunCommand_Hook, pPlayerRunCommand, PlayerRunCommand_Hooked, PlayerRunCommand_Original, PlayerRunCommandFn);
	HOOK_FUNCTION(CheckJumpButtonServer_Hook, pCheckJumpButtonServer, CheckJumpButtonServer_Hooked, CheckJumpButtonServer_Original, CheckJumpButtonServerFn);
	HOOK_FUNCTION(CDirectorSessionManager__FireGameEvent_Hook, pDirectorSessionManager__FireGameEvent, CDirectorSessionManager__FireGameEvent_Hooked, CDirectorSessionManager__FireGameEvent_Original, CDirectorSessionManager__FireGameEventFn);
	HOOK_FUNCTION(RestoreTransitionedEntities_Hook, pRestoreTransitionedEntities, RestoreTransitionedEntities_Hooked, RestoreTransitionedEntities_Original, RestoreTransitionedEntitiesFn);
	HOOK_FUNCTION(TeamStartTouchIsValid_Hook, pTeamStartTouchIsValid, TeamStartTouchIsValid_Hooked, TeamStartTouchIsValid_Original, TeamStartTouchIsValidFn);
	HOOK_FUNCTION(RestartRound_Hook, pRestartRound, RestartRound_Hooked, RestartRound_Original, RestartRoundFn);
	HOOK_FUNCTION(UnfreezeTeam_Hook, pUnfreezeTeam, UnfreezeTeam_Hooked, UnfreezeTeam_Original, UnfreezeTeamFn);
	HOOK_FUNCTION(OnStartIntro_Hook, pOnStartIntro, OnStartIntro_Hooked, OnStartIntro_Original, OnStartIntroFn);
	HOOK_FUNCTION(OnFinishIntro_Hook, pOnFinishIntro, OnFinishIntro_Hooked, OnFinishIntro_Original, OnFinishIntroFn);
	HOOK_FUNCTION(OnBeginTransition_Hook, pOnBeginTransition, OnBeginTransition_Hooked, OnBeginTransition_Original, OnBeginTransitionFn);
	HOOK_FUNCTION(OnFinaleEscapeForceSurvivorPositions_Hook, pOnFinaleEscapeForceSurvivorPositions, OnFinaleEscapeForceSurvivorPositions_Hooked, OnFinaleEscapeForceSurvivorPositions_Original, OnFinaleEscapeForceSurvivorPositionsFn);
	HOOK_FUNCTION(DirectorOnFinaleEscapeForceSurvivorPositions_Hook, pOnFinaleEscapeForceSurvivorPositions2, DirectorOnFinaleEscapeForceSurvivorPositions_Hooked, DirectorOnFinaleEscapeForceSurvivorPositions_Original, DirectorOnFinaleEscapeForceSurvivorPositionsFn);

	m_bInitialized = true;
	return true;
}


bool CServer::Release()
{
	if (!m_bInitialized)
		return false;

	// Reset class member: FnChangeCB_t m_fnChangeCallback
	*reinterpret_cast<DWORD *>(GetOffset(host_timescale, 0x44)) = NULL;

	UNHOOK_FUNCTION(PlayerRunCommand_Hook);
	UNHOOK_FUNCTION(CheckJumpButtonServer_Hook);
	UNHOOK_FUNCTION(CDirectorSessionManager__FireGameEvent_Hook);
	UNHOOK_FUNCTION(RestoreTransitionedEntities_Hook);
	UNHOOK_FUNCTION(TeamStartTouchIsValid_Hook);
	UNHOOK_FUNCTION(RestartRound_Hook);
	UNHOOK_FUNCTION(UnfreezeTeam_Hook);
	UNHOOK_FUNCTION(OnStartIntro_Hook);
	UNHOOK_FUNCTION(OnFinishIntro_Hook);
	UNHOOK_FUNCTION(OnBeginTransition_Hook);
	UNHOOK_FUNCTION(OnFinaleEscapeForceSurvivorPositions_Hook);
	UNHOOK_FUNCTION(DirectorOnFinaleEscapeForceSurvivorPositions_Hook);

	return true;
}

//-----------------------------------------------------------------------------
// Console variables/commands
//-----------------------------------------------------------------------------

CON_COMMAND(autojump_enable, "Enable autojump for a specified player")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: autojump_enable [index]\n");
		return;
	}

	int nIndex = atoi(args[1]);

	IS_PLAYER_INDEX_VALID(nIndex)
	{
		for (int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i)
		{
			C_BasePlayer *pLocal = NULL;

			if (pLocal = g_Client.GetLocalPlayer(i))
			{
				int nLocalPlayerIndex = EntIndexOfBaseEntity((IClientEntity *)pLocal);

				if (nLocalPlayerIndex > 0)
				{
					if (nIndex == nLocalPlayerIndex)
					{
						g_Client.EnableAutoJump(i);
						break;
					}
				}
			}
		}

		g_Server.EnableAutoJump(nIndex);
	}
}

CON_COMMAND(autojump_disable, "Disable autojump for a specified player")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: autojump_enable [index]\n");
		return;
	}

	int nIndex = atoi(args[1]);

	IS_PLAYER_INDEX_VALID(nIndex)
	{
		for (int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i)
		{
			C_BasePlayer *pLocal = NULL;

			if (pLocal = g_Client.GetLocalPlayer(i))
			{
				int nLocalPlayerIndex = EntIndexOfBaseEntity((IClientEntity *)pLocal);

				if (nLocalPlayerIndex > 0)
				{
					if (nIndex == nLocalPlayerIndex)
					{
						g_Client.DisableAutoJump(i);
						break;
					}
				}
			}
		}

		g_Server.DisableAutoJump(nIndex);
	}
}

CON_COMMAND(tas_tp, "Teleport local client to the given position, optional: snap eye angles")
{
	if (args.ArgC() < 4)
	{
		Msg("Usage: tas_tp [x] [y] [z] [optional: pitch] [optional: yaw]\n");
		return;
	}

	edict_t *pEdict = EntIndexToEdict(g_pEngineClient->GetLocalPlayer());

	if (IsEdictValid(pEdict))
	{
		float X = strtof(args.Arg(1), NULL);
		float Y = strtof(args.Arg(2), NULL);
		float Z = strtof(args.Arg(3), NULL);

		QAngle *pViewAngles = NULL;

		Vector vecOrigin = { X, Y, Z };
		Vector vecVelocity = { 0.0f, 0.0f, 0.0f };

		CBaseEntity *pEntity = EdictToBaseEntity(pEdict);

		if (args.ArgC() >= 6)
		{
			float flPitch = strtof(args.Arg(4), NULL);
			float flYaw = strtof(args.Arg(5), NULL);

			QAngle viewangles = { flPitch, flYaw, 0.0f };
			pViewAngles = &viewangles;
		}

		Teleport(pEntity, &vecOrigin, pViewAngles, &vecVelocity);
	}
}

CON_COMMAND(tas_tp_player, "Teleport player to the given position, optional: snap eye angles")
{
	if (!UTIL_PlayerByIndex)
		return;

	if (args.ArgC() < 5)
	{
		Msg("Usage: tas_tp [index] [x] [y] [z] [optional: pitch] [optional: yaw]\n");
		return;
	}

	CBasePlayer *pPlayer = UTIL_PlayerByIndex(atoi(args[1]));

	if (pPlayer)
	{
		float X = strtof(args.Arg(2), NULL);
		float Y = strtof(args.Arg(3), NULL);
		float Z = strtof(args.Arg(4), NULL);

		QAngle *pViewAngles = NULL;

		Vector vecOrigin = { X, Y, Z };
		Vector vecVelocity = { 0.0f, 0.0f, 0.0f };

		if (args.ArgC() >= 7)
		{
			float flPitch = strtof(args.Arg(5), NULL);
			float flYaw = strtof(args.Arg(6), NULL);

			QAngle viewangles = { flPitch, flYaw, 0.0f };
			pViewAngles = &viewangles;
		}

		Teleport((CBaseEntity *)pPlayer, &vecOrigin, pViewAngles, &vecVelocity);
	}
}

CON_COMMAND(tas_setcvar, "Set a value of the given ConVar")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: tas_setcvar [cvar_name] [optional: value]\n");
		return;
	}

	ConVar *pCvar = g_pCVar->FindVar(args.Arg(1));

	if (!pCvar)
	{
		Warning("tas_setcvar: no such ConVar\n");
		return;
	}

	if (args.ArgC() == 2)
		Msg("Value: %s\n", pCvar->GetString());
	else
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
		Warning("timer_set_segment_time: the given value cannot be lower than zero!\n");
}

CON_COMMAND(timer_start, "Start the timer")
{
	g_Timer.Start();

	if (g_pServer->IsPaused())
		g_Timer.OnPreciseTimeCorrupted();

	g_Server.FireServerCallback(CB_TIMER_STARTED, true);
}

CON_COMMAND(timer_stop, "Stop the timer")
{
	g_Timer.Stop();
	g_Server.FireServerCallback(CB_TIMER_STOPPED, true);
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

ConVar timer_auto("timer_auto", "0", FCVAR_RELEASE, "Auto start/end the timer");
ConVar timer_print_to_chat("timer_print_to_chat", "1", FCVAR_RELEASE, "Print time/ticks to chat, otherwise in console");

ConVar tas_vscript_callbacks("tas_vscript_callbacks", "1", FCVAR_RELEASE, "Call VScript callbacks in function Left4TAS_Callbacks");
ConVar tas_autoexec_configs("tas_autoexec_configs", "1", FCVAR_RELEASE, "Autoexec the following files:\n- tas_round_start.cfg\n- tas_map_load.cfg\n- tas_intro_finish.cfg\n- tas_segment_finish.cfg\n- tas_server_transition.cfg\n- tas_map_transition.cfg");
ConVar tas_autorun_vscripts("tas_autorun_vscripts", "1", FCVAR_RELEASE, "Autorun left4tas.nut file before round will start");
ConVar tas_autojump("tas_autojump", "1", FCVAR_REPLICATED | FCVAR_RELEASE, "Player automatically re-jumps while holding jump button");
ConVar tas_timescale("tas_timescale", "1.0", FCVAR_CHEAT | FCVAR_RELEASE | FCVAR_REPLICATED, "Set server's timescale", OnConVarChange);
ConVar tas_im_tp("tas_im_tp", "1", FCVAR_RELEASE, "Teleport player to beginning position");

ConVar category_no_director("category_no_director", "0", FCVAR_CHEAT | FCVAR_RELEASE, "No Director category", OnCategoryChange);
ConVar category_no_survivor_bots("category_no_survivor_bots", "0", FCVAR_CHEAT | FCVAR_RELEASE, "No Survivor Bots category:\n- Modes:\n\t0 - disable\n\t1 - one player\n\t2 - splitscreen", OnCategoryChange);