// Client Module

#pragma comment(lib, "mathlib.lib")

#include "client.h"
#include "engine.h"
#include "server.h"

#include "../cvars.h"
#include "../patterns.h"
#include "../prop_offsets.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "vtable_hook.h"
#include "utils.h"
#include "strafe_utils.h"
#include "usercmd.h"

#include "../libdasm/libdasm.h"

#include "../tools/recvpropmanager.h"
#include "../tools/misc.h"
#include "../tools/strafe.h"
#include "../tools/simple_trigger.h"
#include "../tools/input_manager.h"

#include "../game/cl_splitscreen.h"
#include "game/shared/igamemovement.h"

#ifdef strdup
#undef strdup
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef C_BaseEntity *(__thiscall *GetGroundEntityFn)(void *);

typedef bool (__thiscall *AddSplitScreenUserFn)(void *, int, int);
typedef bool (__thiscall *RemoveSplitScreenUserFn)(void *, int, int);

typedef void (__thiscall *CCSModeManager__InitFn)(void *);

typedef void (__stdcall *HudUpdateFn)(bool); // ecx is not used

typedef bool (__thiscall *IClientMode__CreateMoveFn)(void *, float, CUserCmd *);

typedef int (__thiscall *GetButtonBitsFn)(void *, bool);
typedef void (__thiscall *CreateMoveFn)(void *, int, float, bool);
typedef void (__thiscall *ControllerMoveFn)(void *, int, float, CUserCmd *);
typedef void (__stdcall *AdjustAnglesFn)(int, float); // ecx is not used

typedef bool (__thiscall *CheckJumpButtonClientFn)(IGameMovement *);

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern IBaseClientDLL *g_pClient;
extern IVEngineClient *g_pEngineClient;
extern IClientEntityList *g_pClientEntityList;
extern IServerGameDLL *g_pServerGameDLL;

void OnSetAngle(IConVar *var, const char *pOldValue, float flOldValue);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CClient g_Client;

static bool s_bProcessClientMode = false;
static int s_nActiveSlot = -1;
static int s_lockedSlot = -1;
static uint32_t s_nTicksOnGround = 0;

static CUserCmd *pCmd = NULL;

ISplitScreen *splitscreen = NULL;

//-----------------------------------------------------------------------------
// Native cvars
//-----------------------------------------------------------------------------

ConVar *sv_airaccelerate = NULL;
ConVar *sv_accelerate = NULL;
ConVar *sv_friction = NULL;
ConVar *sv_maxspeed = NULL;
ConVar *sv_stopspeed = NULL;
ConVar *in_forceuser = NULL;
ConVar *cl_mouseenable = NULL;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

VTABLE_HOOK(ISplitScreen_Hook);
VTABLE_HOOK(IBaseClientDLL_Hook);

TRAMPOLINE_HOOK(CCSModeManager__Init_Hook);
TRAMPOLINE_HOOK(CreateMove_Hook);
TRAMPOLINE_HOOK(ControllerMove_Hook);
TRAMPOLINE_HOOK(AdjustAngles_Hook);
TRAMPOLINE_HOOK(CheckJumpButtonClient_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

GetGroundEntityFn GetGroundEntity = NULL;

AddSplitScreenUserFn AddSplitScreenUser_Original = NULL;
RemoveSplitScreenUserFn RemoveSplitScreenUser_Original = NULL;

CCSModeManager__InitFn CCSModeManager__Init_Original = NULL;

HudUpdateFn HudUpdate_Original = NULL;

IClientMode__CreateMoveFn IClientMode__CreateMove_Original = NULL;

CreateMoveFn CreateMove_Original = NULL;
ControllerMoveFn ControllerMove_Original = NULL;
AdjustAnglesFn AdjustAngles_Original = NULL;

CheckJumpButtonClientFn CheckJumpButtonClient_Original = NULL;

//-----------------------------------------------------------------------------
// CQueuedCommand
//-----------------------------------------------------------------------------

CQueuedCommand::CQueuedCommand(long long nFrames, const char *szCommand)
{
	m_nFrames = nFrames;
	m_pszCommand = strdup(szCommand);
}

CQueuedCommand::~CQueuedCommand()
{
	free((void *)m_pszCommand);
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

bool __fastcall AddSplitScreenUser_Hooked(void *thisptr, int edx, int nSlot, int nPlayerIndex)
{
	bool result = AddSplitScreenUser_Original(thisptr, nSlot, nPlayerIndex);

	if (result)
		g_Client.m_bInSplitScreen = true;

	return result;
}

bool __fastcall RemoveSplitScreenUser_Hooked(void *thisptr, int edx, int nSlot, int nPlayerIndex)
{
	bool result = RemoveSplitScreenUser_Original(thisptr, nSlot, nPlayerIndex);

	if (result)
		g_Client.m_bInSplitScreen = false;

	return result;
}

void __stdcall HudUpdate_Hooked(bool bActive)
{
	HudUpdate_Original(bActive);

	g_Client.ProcessTriggers();
}

bool __fastcall IClientMode__CreateMove_Hooked(void *thisptr, int edx, float flInputSampleTime, CUserCmd *cmd)
{
	bool overridden = false;
	bool result = IClientMode__CreateMove_Original(thisptr, flInputSampleTime, cmd);

	InputData *pInputData = g_InputManager.GetClientInputData(s_nActiveSlot);

	// Input Manager
	if (s_bProcessClientMode && pInputData->input)
	{
		if (pInputData->recording)
		{
			g_InputManager.SaveInput(pInputData, cmd, true);
		}
		else
		{
			g_InputManager.ReadInput(pInputData, cmd, g_Client.GetLocalPlayer(s_nActiveSlot), true);
			overridden = true;
		}

		++pInputData->frames;
	}

	s_bProcessClientMode = false;

	return result || overridden;
}

void __fastcall CreateMove_Hooked(void *thisptr, int edx, int sequence_number, float input_sample_frametime, bool active)
{
	// void *pPerUser = &(reinterpret_cast<DWORD *>(thisptr)[13]); // this[0x2F * nSlot + 13]
	// DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(pPerUser, Offsets::Variables::m_pCommands = 168));
	// 168 + 13 * 4 = 220 = 0xDC

	//DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(thisptr, Offsets::Variables::m_pCommands));

	g_Client.m_nForceUser = in_forceuser->GetInt();
	s_nActiveSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
	s_bProcessClientMode = true;

	void *pPerUser = &reinterpret_cast<DWORD *>(thisptr)[0x2F * s_nActiveSlot + 13];
	DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(pPerUser, Offsets::Variables::m_pCommands));

	pCmd = reinterpret_cast<CUserCmd *>(m_pCommands + SIZEOF_USERCMD * (sequence_number % MULTIPLAYER_BACKUP));

	if (ss_forceuser.GetBool())
		in_forceuser->SetValue(s_nActiveSlot);

	CreateMove_Original(thisptr, sequence_number, input_sample_frametime, active);

	in_forceuser->SetValue(g_Client.m_nForceUser);
	pCmd = NULL;

	g_Client.ProcessQueuedCommands();
}

void __fastcall ControllerMove_Hooked(void *thisptr, int edx, int nSlot, float frametime, CUserCmd *cmd)
{
	int nEnableMouse = cl_mouseenable->GetInt();

	// Don't let a player control the camera angles of another player
	if (ss_forceuser.GetBool() && g_Client.m_nForceUser != nSlot)
		cl_mouseenable->SetValue(0);

	ControllerMove_Original(thisptr, nSlot, frametime, cmd);

	cl_mouseenable->SetValue(nEnableMouse);
}

void __stdcall AdjustAngles_Hooked(int nSlot, float frametime)
{
	AdjustAngles_Original(nSlot, frametime);

	if (pCmd)
		g_Client.ProcessInputs(nSlot);
}

// Called multiple times when you have ping, this game sucks
bool __fastcall CheckJumpButtonClient_Hooked(IGameMovement *thisptr)
{
	bool bJumped = g_Client.CheckJumpButton(thisptr);

	return bJumped;
}

void __fastcall CCSModeManager__Init_Hooked(void *thisptr)
{
	CCSModeManager__Init_Original(thisptr);

	if (IClientMode__CreateMove_Original)
		return;

	IClientMode__CreateMove_Original = (IClientMode__CreateMoveFn)CVTableHook::HookFunction(g_Client.GetClientMode(0), IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
	CVTableHook::HookFunction(g_Client.GetClientMode(1), IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
}

//-----------------------------------------------------------------------------
// Client module implementations
//-----------------------------------------------------------------------------

void CClient::UpdateStrafeData(int nSlot)
{
	C_BasePlayer *pLocal = m_pLocalPlayer[nSlot];

	*reinterpret_cast<Vector *>(m_StrafeData[nSlot].player.Velocity) = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecVelocity));
	*reinterpret_cast<Vector *>(m_StrafeData[nSlot].player.Origin) = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecOrigin));

	m_StrafeData[nSlot].vars.OnGround = GetGroundEntity(pLocal) != NULL;
	m_StrafeData[nSlot].vars.EntFriction = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flFriction));
	m_StrafeData[nSlot].vars.ReduceWishspeed = m_StrafeData[nSlot].vars.OnGround && (*reinterpret_cast<int *>(GetOffset(pLocal, RecvPropOffsets::m_fFlags)) & FL_DUCKING);

	m_StrafeData[nSlot].vars.Maxspeed = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flMaxspeed));
	m_StrafeData[nSlot].vars.Stopspeed = sv_stopspeed->GetFloat();
	m_StrafeData[nSlot].vars.Friction = sv_friction->GetFloat();

	m_StrafeData[nSlot].vars.Accelerate = sv_accelerate->GetFloat();
	m_StrafeData[nSlot].vars.Airaccelerate = sv_airaccelerate->GetFloat();
}

void CClient::GetViewAngles(int nSlot, QAngle &va)
{
	CClientState *pClient = GetLocalClient(nSlot);
	va = *reinterpret_cast<QAngle *>(GetOffset(pClient, Offsets::Variables::m_Client__viewangles));
}

void CClient::SetViewAngles(int nSlot, QAngle &va)
{
	CClientState *pClient = GetLocalClient(nSlot);

	if (va.IsValid())
	{
		*reinterpret_cast<QAngle *>(GetOffset(pClient, Offsets::Variables::m_Client__viewangles)) = va;
	}
	else
	{
		Warning("CEngineClient::SetViewAngles:  rejecting invalid value [%f %f %f]\n", VectorExpand(va));
		*reinterpret_cast<QAngle *>(GetOffset(pClient, Offsets::Variables::m_Client__viewangles)) = vec3_angle;
	}
}

void CClient::ExecuteClientCmd(int nSlot, const char *pszCommand)
{
	g_Engine.Cbuf_AddText(static_cast<ECommandTarget_t>(nSlot), pszCommand, kCommandSrcCode);
	g_Engine.Cbuf_Execute();
}

void CClient::ExecuteClientCmd(ECommandTarget_t target, const char *pszCommand)
{
	g_Engine.Cbuf_AddText(target, pszCommand, kCommandSrcCode);
	g_Engine.Cbuf_Execute();
}

bool CClient::ChangeAngle(float &flAngle, float flTargetAngle)
{
	float normalizedDiff = Strafe::NormalizeDeg(static_cast<double>(flTargetAngle) - flAngle);

	if (std::abs(normalizedDiff) > tas_setanglespeed.GetFloat())
	{
		flAngle += std::copysign(tas_setanglespeed.GetFloat(), normalizedDiff);
		return true;
	}
	else
	{
		flAngle = flTargetAngle;
		return false;
	}
}

bool CClient::ChangeAngleBySpeed(float &flAngle, float flTargetAngle, float flChangeSpeed)
{
	float normalizedDiff = Strafe::NormalizeDeg(static_cast<double>(flTargetAngle) - flAngle);

	if (std::abs(normalizedDiff) > flChangeSpeed)
	{
		flAngle += std::copysign(flChangeSpeed, normalizedDiff);
		return true;
	}
	else
	{
		flAngle = flTargetAngle;
		return false;
	}
}

void CClient::CreateTrigger(const char *pszExecFile, Vector &vecStart, Vector &vecEnd, int iData)
{
	CSimpleTrigger *pTrigger = new CSimpleTrigger(pszExecFile, vecStart, vecEnd);
	pTrigger->SetCustomData(iData);

	m_vecTriggers.push_back(pTrigger);
}

void CClient::RemoveTriggers()
{
	for (size_t i = 0; i < m_vecTriggers.size(); ++i)
		delete m_vecTriggers[i];

	m_vecTriggers.clear();
}

void CClient::RemoveTriggersForClient(int nSlot)
{
	if (nSlot == 0)
	{
		for (size_t i = 0; i < m_vecTriggers.size(); ++i)
		{
			if (m_vecTriggers[i]->GetCustomData() == 1)
			{
				delete m_vecTriggers[i];
				m_vecTriggers.erase(m_vecTriggers.begin() + i);
				--i;
			}
		}
	}
	else /* if (nSlot == 1) */
	{
		for (size_t i = 0; i < m_vecTriggers.size(); ++i)
		{
			if (m_vecTriggers[i]->GetCustomData() == 2)
			{
				delete m_vecTriggers[i];
				m_vecTriggers.erase(m_vecTriggers.begin() + i);
				--i;
			}
		}
	}
}

void CClient::AddQueuedCommand(long long nFrames, const char *pszCommand)
{
	CQueuedCommand *queued_cmd = new CQueuedCommand(nFrames, pszCommand);
	m_vecQueuedCommands[GET_ACTIVE_SPLITSCREEN_SLOT()].push_back(queued_cmd);
}

void CClient::AddQueuedCommand(int nSlot, long long nFrames, const char *pszCommand)
{
	CQueuedCommand *queued_cmd = new CQueuedCommand(nFrames, pszCommand);
	m_vecQueuedCommands[nSlot].push_back(queued_cmd);
}

void CClient::ClearQueuedCommands()
{
	for (size_t i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i)
	{
		for (size_t j = 0; j < m_vecQueuedCommands[i].size(); ++j)
			delete m_vecQueuedCommands[i][j];

		m_vecQueuedCommands[i].clear();
	}
}

void CClient::ClearQueuedCommands(int nSlot)
{
	for (size_t i = 0; i < m_vecQueuedCommands[nSlot].size(); ++i)
		delete m_vecQueuedCommands[nSlot][i];

	m_vecQueuedCommands[nSlot].clear();
}

void CClient::ProcessTriggers()
{
	Vector *vecOrigin = NULL; // splitscreen slot 0
	Vector *vecOrigin_SS = NULL; // slot 1

	if (m_pLocalPlayer[0])
		vecOrigin = reinterpret_cast<Vector *>(GetOffset(m_pLocalPlayer[0], RecvPropOffsets::m_vecOrigin));

	if (m_pLocalPlayer[1])
		vecOrigin_SS = reinterpret_cast<Vector *>(GetOffset(m_pLocalPlayer[1], RecvPropOffsets::m_vecOrigin));

	for (size_t i = 0; i < m_vecTriggers.size(); ++i)
	{
		CSimpleTrigger *trigger = m_vecTriggers[i];

		switch (trigger->GetCustomData())
		{
		case 0:
			if (vecOrigin && trigger->Think(vecOrigin))
				goto REMOVE_TRIGGER;

			if (vecOrigin_SS && trigger->Think(vecOrigin_SS))
				goto REMOVE_TRIGGER;

			break;

		case 1:
			if (vecOrigin && trigger->Think(vecOrigin))
				goto REMOVE_TRIGGER;

			break;

		case 2:
			if (vecOrigin_SS && trigger->Think(vecOrigin_SS))
				goto REMOVE_TRIGGER;

			break;

		default:
			break;
		}

		continue;

	REMOVE_TRIGGER:
		delete trigger;

		m_vecTriggers.erase(m_vecTriggers.begin() + i);
		--i;
	}
}

void CClient::ProcessQueuedCommands()
{
	if (wait_frames_pause.GetBool())
		return;

	if (!s_nActiveSlot) // process commands of the first client
	{
		for (size_t i = 0; i < m_vecQueuedCommands[0].size(); ++i)
		{
			CQueuedCommand *queued_cmd = m_vecQueuedCommands[0][i];

			if (--(queued_cmd->m_nFrames) <= 0)
			{
				ExecuteClientCmd(s_nActiveSlot, queued_cmd->m_pszCommand);
				delete queued_cmd;

				m_vecQueuedCommands[0].erase(m_vecQueuedCommands[0].begin() + i);
				--i;
			}
		}
	}
	else // process commands of the second client
	{
		for (size_t i = 0; i < m_vecQueuedCommands[1].size(); ++i)
		{
			CQueuedCommand *queued_cmd = m_vecQueuedCommands[1][i];

			if (--(queued_cmd->m_nFrames) <= 0)
			{
				ExecuteClientCmd(s_nActiveSlot, queued_cmd->m_pszCommand);
				delete queued_cmd;

				m_vecQueuedCommands[1].erase(m_vecQueuedCommands[1].begin() + i);
				--i;
			}
		}
	}
}

void CClient::ProcessInputs(int nSlot)
{
	QAngle viewangles;
	g_pEngineClient->GetViewAngles(viewangles);

	InputData *pInputData = g_InputManager.GetClientInputData(nSlot);

	bool bYawChanged = false;
	bool bOnGround = false;

	// Angle change
	if (!pInputData->input && !pInputData->recording)
	{
		if (m_InputAction[nSlot].m_bSetAngles)
		{
			bool pitchChanged = ChangeAngleBySpeed(viewangles[PITCH], m_InputAction[nSlot].m_vecSetAngles[PITCH], m_InputAction[nSlot].m_vecSetAnglesSpeed[PITCH]);
			bool yawChanged = bYawChanged = ChangeAngleBySpeed(viewangles[YAW], m_InputAction[nSlot].m_vecSetAngles[YAW], m_InputAction[nSlot].m_vecSetAnglesSpeed[YAW]);

			if (!pitchChanged && !yawChanged)
				m_InputAction[nSlot].m_bSetAngles = false;
		}
		else
		{
			if (m_InputAction[nSlot].m_bSetPitch)
				m_InputAction[nSlot].m_bSetPitch = ChangeAngle(viewangles[PITCH], tas_setpitch.GetFloat());

			if (m_InputAction[nSlot].m_bSetYaw)
				m_InputAction[nSlot].m_bSetYaw = bYawChanged = ChangeAngle(viewangles[YAW], tas_setyaw.GetFloat());
		}
	}

	// Strafe
	if (m_StrafeData[nSlot].frame.Strafe && (!pInputData->input || pInputData->recording))
	{
		UpdateStrafeData(nSlot);

		if (!m_StrafeData[nSlot].frame.IgnoreGround || !m_StrafeData[nSlot].vars.OnGround)
		{
			Strafe::ProcessedFrame out;
			out.Yaw = viewangles[YAW];

			if (m_StrafeData[nSlot].frame.StrafeToViewAngles)
			{
				if (m_StrafeData[nSlot].frame.StrafeVectorial)
					m_StrafeData[nSlot].frame.SetYaw(viewangles[YAW]);
				else
					m_StrafeData[nSlot].frame.SetYaw(0.0);
			}

			Strafe::Friction(m_StrafeData[nSlot]);

			if (m_StrafeData[nSlot].frame.StrafeVectorial)
				Strafe::StrafeVectorial(m_StrafeData[nSlot], out, bYawChanged);
			else if (!bYawChanged)
				Strafe::Strafe(m_StrafeData[nSlot], out);

			if (out.Processed)
			{
				pCmd->forwardmove = out.Forwardspeed;
				pCmd->sidemove = out.Sidespeed;

				viewangles[YAW] = static_cast<float>(out.Yaw);
			}
		}
	}

	if (m_nForceUser == nSlot && GetGroundEntity(m_pLocalPlayer[nSlot]))
		++s_nTicksOnGround;

	g_pEngineClient->SetViewAngles(viewangles);
}

bool CClient::CheckJumpButton(IGameMovement *pGameMovement)
{
	int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	CMoveData *mv = reinterpret_cast<CMoveData *>(((uintptr_t *)pGameMovement)[2]);
	const uint IN_JUMP = mv->m_nOldButtons & (1 << 1);

	if (tas_autojump.GetBool() && m_bAutoJump[nSlot])
		mv->m_nOldButtons &= ~IN_JUMP;

	bool bJumped = CheckJumpButtonClient_Original(pGameMovement);

	if (bJumped)
	{
		int nForceUser = in_forceuser->GetInt();

		if (nSlot == nForceUser)
		{
			float flCurrentSpeed = reinterpret_cast<Vector *>(GetOffset(m_pLocalPlayer[nForceUser], RecvPropOffsets::m_vecVelocity))->Length2D();

			if (s_nTicksOnGround <= 3)
			{
				float flLoss = m_BunnyhopInfo.m_flLastSpeed - flCurrentSpeed;

				m_BunnyhopInfo.m_flSpeedLoss = flLoss > 0.0f ? flLoss : 0.0f;
				m_BunnyhopInfo.m_flPercentage = (flCurrentSpeed / m_BunnyhopInfo.m_flLastSpeed) * 100.0f;

				if (m_BunnyhopInfo.m_flPercentage > 1000000.0f)
					m_BunnyhopInfo.m_flPercentage = 1000000.0f;

				++m_BunnyhopInfo.m_nJumps;
			}
			else
			{
				m_BunnyhopInfo.m_nJumps = 0;
				m_BunnyhopInfo.m_flSpeedLoss = 0.0f;
				m_BunnyhopInfo.m_flPercentage = 100.0f;
			}

			m_BunnyhopInfo.m_flLastSpeed = flCurrentSpeed;
			s_nTicksOnGround = 0;
		}
	}

	mv->m_nOldButtons |= IN_JUMP;
	return bJumped;
}

CClient::CClient()
{
	m_bInitialized = false;
	m_bAutoJump[0] = m_bAutoJump[1] = true;

	m_nForceUser = 0;
	m_bInSplitScreen = false;

	m_pLocalPlayer =  NULL;
	m_pClientMode = NULL;
	m_pSplitScreenPlayers = NULL;
}

bool CClient::IsInitialized() const
{
	return m_bInitialized;
}

bool CClient::Init()
{
	INSTRUCTION instruction;

	if (!g_Engine.IsInitialized())
		return false;

	// m_pLocalPlayer
	void *pCheckForLocalPlayer = FIND_PATTERN(L"client.dll", Patterns::Client::C_BasePlayer__CheckForLocalPlayer);

	if (!pCheckForLocalPlayer)
	{
		FailedInit("C_BasePlayer::CheckForLocalPlayer");
		return false;
	}
	
	void *pCreateMove = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__CreateMove);

	if (!pCreateMove)
	{
		FailedInit("CInput::CreateMove");
		return false;
	}
	
	void *pControllerMove = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__ControllerMove);

	if (!pControllerMove)
	{
		FailedInit("CInput::ControllerMove");
		return false;
	}

	void *pAdjustAngles = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__AdjustAngles);

	if (!pAdjustAngles)
	{
		FailedInit("CInput::AdjustAngles");
		return false;
	}

	void *pCheckJumpButtonClient = FIND_PATTERN(L"client.dll", Patterns::Client::CGameMovement__CheckJumpButton);

	if (!pCheckJumpButtonClient)
	{
		FailedInit("CGameMovement::CheckJumpButton");
		return false;
	}
	
	void *pCCSModeManager__Init = FIND_PATTERN(L"client.dll", Patterns::Client::CCSModeManager__Init);

	if (!pCCSModeManager__Init)
	{
		FailedInit("CCSModeManager::Init");
		return false;
	}
	else if (*(BYTE *)(pCCSModeManager__Init = GetOffset(pCCSModeManager__Init, Offsets::Functions::CCSModeManager__Init)) != 0x55) // prologue
	{
		FailedInit("CCSModeManager::Init (can't find function prologue)");
		return false;
	}

	if (!g_Server.IsInitialized())
		m_bAutoJump[0] = m_bAutoJump[1] = false;

	// IClientMode *m_pClientMode[MAX_SPLITSCREEN_PLAYERS];
	void *pOpenSelection_String = NULL;
	void *pCHudWeaponSelection__OpenSelection_String = LookupForString(L"client.dll", "OpenWe"); // OpenWeaponSelectionMenu

	if (pCHudWeaponSelection__OpenSelection_String && (pOpenSelection_String = FindAddress(L"client.dll", pCHudWeaponSelection__OpenSelection_String)))
	{
		BYTE *pClientMode = (BYTE *)pOpenSelection_String;

		while (*--pClientMode != 0xE8) { } // CALL opcode

		pClientMode = (BYTE *)GetOffset(GetFunctionAddress(pClientMode), Offsets::Variables::g_pClientMode);

		get_instruction(&instruction, pClientMode, MODE_32);

		if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
		{
			m_pClientMode = reinterpret_cast<IClientMode **>(instruction.op2.displacement);
		}
		else
		{
			FailedIFace("IClientMode");
			return false;
		}
	}
	else
	{
		FailedInit("CHudWeaponSelection::OpenSelection");
		return false;
	}

	// ISplitScreen
	get_instruction(&instruction, (BYTE *)GetVTableFunction(g_pEngineClient, Offsets::Functions::IVEngineClient__GetActiveSplitScreenPlayerSlot), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		splitscreen = *reinterpret_cast<ISplitScreen **>(instruction.op2.displacement);
	}
	else
	{
		FailedIFace("ISplitScreen");
		return false;
	}
	
	// SplitPlayer_t *m_pSplitScreenPlayers[MAX_SPLITSCREEN_PLAYERS];
	get_instruction(&instruction, (BYTE *)g_Engine.GetBaseLocalClientFunc(), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		m_pSplitScreenPlayers = reinterpret_cast<SplitPlayer_t **>(instruction.op2.displacement);
	}
	else
	{
		FailedInit("m_SplitScreenPlayers");
		return false;
	}

	// C_BasePlayer *m_pLocalPlayer[MAX_SPLITSCREEN_PLAYERS];
	get_instruction(&instruction, (BYTE *)GetOffset(pCheckForLocalPlayer, Offsets::Variables::s_pLocalPlayer), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		m_pLocalPlayer = reinterpret_cast<C_BasePlayer **>(instruction.op2.displacement);
	}
	else
	{
		FailedInit("s_pLocalPlayer");
		return false;
	}

	// C_BaseEntity::GetGroundEntity
	void *pGetGroundEntityCall = GetOffset(pCheckJumpButtonClient, Offsets::Functions::C_BaseEntity__GetGroundEntity);

	if (*(BYTE *)pGetGroundEntityCall == 0xE8) // CALL opcode
	{
		GetGroundEntity = (GetGroundEntityFn)GetFunctionAddress(pGetGroundEntityCall);
	}
	else
	{
		FailedInit("C_BaseEntity::GetGroundEntity");
		return false;
	}

	// Get offsets of receive properties
	RecvPropOffsets::m_fFlags = RecvProps.GetPropOffset("CBasePlayer", "m_fFlags");
	RecvPropOffsets::m_vecVelocity = RecvProps.GetPropOffset("CBasePlayer", "m_vecVelocity[0]");
	RecvPropOffsets::m_vecOrigin = RecvProps.GetPropOffset("CBaseEntity", "m_vecOrigin");
	RecvPropOffsets::m_flMaxspeed = RecvProps.GetPropOffset("CBasePlayer", "m_flMaxspeed");
	RecvPropOffsets::m_flFriction = RecvProps.GetPropOffset("CBasePlayer", "m_flFriction");
	RecvPropOffsets::m_hMyWeapons = RecvProps.GetPropOffset("CBasePlayer", "m_hMyWeapons");

	// Native cvars
	sv_airaccelerate = g_pCVar->FindVar("sv_airaccelerate");
	sv_accelerate = g_pCVar->FindVar("sv_accelerate");
	sv_friction = g_pCVar->FindVar("sv_friction");
	sv_maxspeed = g_pCVar->FindVar("sv_maxspeed");
	sv_stopspeed = g_pCVar->FindVar("sv_stopspeed");
	in_forceuser = g_pCVar->FindVar("in_forceuser");
	cl_mouseenable = g_pCVar->FindVar("cl_mouseenable");

	m_nForceUser = in_forceuser->GetInt();
	m_StrafeData[0].vars.Frametime = m_StrafeData[1].vars.Frametime = g_pServerGameDLL->GetTickInterval();

	// IClientMode hooks
	if (m_pClientMode[0] && m_pClientMode[1])
	{
		IClientMode__CreateMove_Original = (IClientMode__CreateMoveFn)CVTableHook::HookFunction(m_pClientMode[0], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
		CVTableHook::HookFunction(m_pClientMode[1], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
	}

	// Trampoline hook
	HOOK_FUNCTION(CCSModeManager__Init_Hook, pCCSModeManager__Init, CCSModeManager__Init_Hooked, CCSModeManager__Init_Original, CCSModeManager__InitFn);
	HOOK_FUNCTION(CreateMove_Hook, pCreateMove, CreateMove_Hooked, CreateMove_Original, CreateMoveFn);
	HOOK_FUNCTION(ControllerMove_Hook, pControllerMove, ControllerMove_Hooked, ControllerMove_Original, ControllerMoveFn);
	HOOK_FUNCTION(AdjustAngles_Hook, pAdjustAngles, AdjustAngles_Hooked, AdjustAngles_Original, AdjustAnglesFn);
	HOOK_FUNCTION(CheckJumpButtonClient_Hook, pCheckJumpButtonClient, CheckJumpButtonClient_Hooked, CheckJumpButtonClient_Original, CheckJumpButtonClientFn);

	// Hook vtable
	HOOK_VTABLE_AUTOGUESS(ISplitScreen_Hook, splitscreen);
	HOOK_VTABLE(IBaseClientDLL_Hook, g_pClient, Offsets::Functions::IBaseClientDLL__HudUpdate + 1);

	// Hook vtable function(s)
	HOOK_VTABLE_FUNC(ISplitScreen_Hook, AddSplitScreenUser_Hooked, Offsets::Functions::ISplitScreen__AddSplitScreenUser, AddSplitScreenUser_Original, AddSplitScreenUserFn);
	HOOK_VTABLE_FUNC(ISplitScreen_Hook, RemoveSplitScreenUser_Hooked, Offsets::Functions::ISplitScreen__RemoveSplitScreenUser, RemoveSplitScreenUser_Original, RemoveSplitScreenUserFn);
	HOOK_VTABLE_FUNC(IBaseClientDLL_Hook, HudUpdate_Hooked, Offsets::Functions::IBaseClientDLL__HudUpdate, HudUpdate_Original, HudUpdateFn);

	m_bInitialized = true;
	return true;
}

bool CClient::Release()
{
	if (!m_bInitialized)
		return false;

	if (IClientMode__CreateMove_Original && m_pClientMode[0] && m_pClientMode[1])
	{
		CVTableHook::UnhookFunction(m_pClientMode[0], IClientMode__CreateMove_Original, Offsets::Functions::IClientMode__CreateMove);
		CVTableHook::UnhookFunction(m_pClientMode[1], IClientMode__CreateMove_Original, Offsets::Functions::IClientMode__CreateMove);
	}

	// Unhook functions
	UNHOOK_FUNCTION(CCSModeManager__Init_Hook);
	UNHOOK_FUNCTION(CreateMove_Hook);
	UNHOOK_FUNCTION(ControllerMove_Hook);
	UNHOOK_FUNCTION(AdjustAngles_Hook);
	UNHOOK_FUNCTION(CheckJumpButtonClient_Hook);

	// Unhook vtable functions
	UNHOOK_VTABLE_FUNC(ISplitScreen_Hook, Offsets::Functions::ISplitScreen__AddSplitScreenUser);
	UNHOOK_VTABLE_FUNC(ISplitScreen_Hook, Offsets::Functions::ISplitScreen__RemoveSplitScreenUser);
	UNHOOK_VTABLE_FUNC(IBaseClientDLL_Hook, Offsets::Functions::IBaseClientDLL__HudUpdate);

	// Remove vtable hook
	REMOVE_VTABLE_HOOK(ISplitScreen_Hook);
	REMOVE_VTABLE_HOOK(IBaseClientDLL_Hook);

	return true;
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(LOCK_SPLITSCREEN_SLOT, "Lock the given splitscreen slot as active")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: LOCK_SPLITSCREEN_SLOT [slot]\n");
		return;
	}

	int nSlot = atoi(args[1]);

	if (IS_VALID_SPLIT_SCREEN_SLOT(nSlot))
	{
		s_lockedSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
		SET_ACTIVE_SPLIT_SCREEN_PLAYER_SLOT(nSlot);
	}
}

CON_COMMAND(UNLOCK_SPLITSCREEN_SLOT, "Unlock the previous splitscreen slot as active")
{
	if (s_lockedSlot != -1)
	{
		if (IS_VALID_SPLIT_SCREEN_SLOT(s_lockedSlot))
			SET_ACTIVE_SPLIT_SCREEN_PLAYER_SLOT(s_lockedSlot);

		s_lockedSlot = -1;
	}
}

CON_COMMAND(ss_map2, "Start a map with splitscreen mode")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: ss_map2 [mapname]\n");
		return;
	}

	if (strlen(args[1]) > 80)
	{
		Warning("ss_map2: buffer overflow\n");
		return;
	}

	static char map_buffer[128];
	sprintf(map_buffer, "map %s;wait 15;connect_splitscreen localhost 2", args[1]);

	g_Client.ExecuteClientCmd(CBUF_FIRST_PLAYER, map_buffer);
}

CON_COMMAND(ss_cmd, "Send a command to a splitscreen player")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: ss_cmd [command]\n");
		return;
	}

	// If we're processing in_forceuser, then send command to another splitscreen player
	// Our ss slot is 0, then 0 ^ 1 = 1. Our ss slot is 1, then 1 ^ 1 = 0
	int nSlot = in_forceuser->GetInt() ^ 1;

	if (IS_VALID_SPLIT_SCREEN_SLOT(nSlot))
		g_Client.ExecuteClientCmd(nSlot, args.Arg(1));
}

CON_COMMAND(autojump, "Toggle autojump for local player")
{
	if (g_Server.IsInitialized())
	{
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
		int nPlayerIndex = g_pEngineClient->GetLocalPlayer();

		g_Client.ToggleAutoJump(nSlot);
		g_Server.ToggleAutoJump(nPlayerIndex);
	}
}

CON_COMMAND(wait_frames, "Execute a given command after a certain number of game frames")
{
	if (args.ArgC() < 3)
	{
		Msg("Usage: wait_frames [count] [command] [optional: splitscreen player]\n");
		return;
	}

	int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	long long nFrames = atoll(args.Arg(1));
	const char *pszCommand = args.Arg(2);

	if (*pszCommand)
	{
		if (nFrames <= 0)
		{
			if (nSlot == 0 && args.ArgC() == 3)
				g_Client.ExecuteClientCmd(0, pszCommand);
			else if (IS_VALID_SPLIT_SCREEN_SLOT(1))
				g_Client.ExecuteClientCmd(1, pszCommand);
		}
		else
		{
			g_Client.AddQueuedCommand((nSlot == 0 && args.ArgC() == 3) ? 0 : 1, nFrames, pszCommand);
		}
	}
}

CON_COMMAND(wait_frames_clear_queue, "Clear the queue of wait_frames")
{
	if (args.ArgC() == 1)
	{
		g_Client.ClearQueuedCommands();
	}
	else if (args.ArgC() >= 2)
	{
		if (atoi(args.Arg(1)) == 0)
			g_Client.ClearQueuedCommands(0);
		else
			g_Client.ClearQueuedCommands(1);
	}
}

CON_COMMAND(tas_create_trigger, "Create a trigger for listening local player's position in the given box coordinates")
{
	if (args.ArgC() < 8)
	{
		Msg("Usage: tas_create_trigger [name of exec. file] [start: x] [start: y] [start: z] [end: x] [end: y] [end: z] [optional: mode (0 - 2)]\n");
		return;
	}

	if (strlen(args[1]) + 1 > TRIGGER_OUTPUT_MAXLEN)
	{
		Msg("tas_create_trigger: buffer overflow\n");
		return;
	}

	int mode = 0;

	Vector buffer;

	Vector vecStart = { static_cast<float>(atof(args[2])), static_cast<float>(atof(args[3])), static_cast<float>(atof(args[4])) };
	Vector vecEnd = { static_cast<float>(atof(args[5])), static_cast<float>(atof(args[6])), static_cast<float>(atof(args[7])) };

	if (vecStart.x > vecEnd.x || vecStart.y > vecEnd.y || vecStart.z > vecEnd.z)
		M_SWAP(vecStart, vecEnd, buffer);

	if (args.ArgC() >= 9)
	{
		mode = atoi(args[8]);
		mode = clamp(mode, 0, 2);
	}

	g_Client.CreateTrigger(args[1], vecStart, vecEnd, mode);
}

CON_COMMAND(tas_remove_triggers, "Remove all triggers")
{
	if (args.ArgC() == 1)
	{
		g_Client.RemoveTriggers();
	}
	else if (args.ArgC() >= 2)
	{
		if (atoi(args.Arg(1)) == 0)
			g_Client.RemoveTriggersForClient(0);
		else
			g_Client.RemoveTriggersForClient(1);
	}
}

CON_COMMAND(tas_reset_angles_change, "Reset change of angles")
{
	for (int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i)
	{
		CInputAction &input_action = g_Client.GetInputAction(i);

		input_action.m_bSetAngles = false;
		input_action.m_bSetPitch = false;
		input_action.m_bSetYaw = false;
	}
}

CON_COMMAND(tas_setangles, "Set pitch and yaw angles")
{
	if (args.ArgC() != 4)
	{
		Msg("Usage: tas_setangles [pitch] [yaw] [frames]\n");
		return;
	}

	int nFrames = atoi(args.Arg(3));

	if (nFrames > 0)
	{
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

		QAngle viewangles;
		g_pEngineClient->GetViewAngles(viewangles);

		CInputAction &input_action = g_Client.GetInputAction(nSlot);

		float flPitch = atof(args.Arg(1));
		float flYaw = atof(args.Arg(2));

		float flNormalizedPitch = Strafe::NormalizeDeg(static_cast<double>(flPitch) - viewangles[PITCH]);
		float flNormalizedYaw = Strafe::NormalizeDeg(static_cast<double>(flYaw) - viewangles[YAW]);

		input_action.m_vecSetAngles[PITCH] = flPitch;
		input_action.m_vecSetAngles[YAW] = flYaw;

		input_action.m_vecSetAnglesSpeed[PITCH] = std::abs(flNormalizedPitch) / nFrames;
		input_action.m_vecSetAnglesSpeed[YAW] = std::abs(flNormalizedYaw) / nFrames;

		input_action.m_bSetAngles = true;
	}
}

CON_COMMAND(tas_setpitch_now, "Set pitch angle in this frame")
{
	if (args.ArgC() != 2)
	{
		Msg("Usage: tas_setpitch_now [pitch]\n");
		return;
	}

	int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	QAngle viewangles;
	g_pEngineClient->GetViewAngles(viewangles);

	viewangles[PITCH] = strtof(args[1], NULL);

	g_pEngineClient->SetViewAngles(viewangles);
}

CON_COMMAND(tas_setyaw_now, "Set yaw angle in this frame")
{
	if (args.ArgC() != 2)
	{
		Msg("Usage: tas_setyaw_now [yaw]\n");
		return;
	}

	int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	QAngle viewangles;
	g_pEngineClient->GetViewAngles(viewangles);

	viewangles[YAW] = strtof(args[1], NULL);

	g_pEngineClient->SetViewAngles(viewangles);
}

// Strafing Tools

CON_COMMAND(tas_strafe, "Enable TAS strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe [0/1]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.Strafe);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.Strafe = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_dir, "TAS strafe directions:\n\t0 - to the left\n\t1 - to the right\n\t2 - best strafe\n\t3 - to the yaw given in tas_strafe_yaw\n\t4 - to the point given in tas_strafe_point")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_dir [0-4]\n", static_cast<int>(g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.GetDir()));
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.SetDir(static_cast<Strafe::StrafeDir>(atoi(args[1])));
}

CON_COMMAND(tas_strafe_type, "TAS strafe types:\n\t0 - Max acceleration strafing\n\t1 - Max angle strafing\n\t2 - Max deceleration strafing\n\t3 - Const speed strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_type [0-3]\n", static_cast<int>(g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.GetType()));
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.SetType(static_cast<Strafe::StrafeType>(atoi(args[1])));
}

CON_COMMAND(tas_strafe_point, "2D point to strafe with tas_strafe_dir = 4")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();

	if (args.ArgC() < 3)
	{
		Msg("Value: { %f, %f } | Usage: tas_strafe_point [x] [y]\n", g_Client.GetStrafeData(slot).frame.GetX(), g_Client.GetStrafeData(slot).frame.GetY());
		return;
	}

	g_Client.GetStrafeData(slot).frame.SetX(strtof(args[1], NULL));
	g_Client.GetStrafeData(slot).frame.SetY(strtof(args[2], NULL));
}

CON_COMMAND(tas_strafe_yaw, "Yaw to strafe to with tas_strafe_dir = 3")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();

	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_yaw [yaw]\n", g_Client.GetStrafeData(slot).frame.StrafeToViewAngles ? 0.0f : g_Client.GetStrafeData(slot).frame.GetYaw());
		return;
	}

	const char *pszYaw = args[1];

	if (*pszYaw)
	{
		g_Client.GetStrafeData(slot).frame.SetYaw(strtof(args[1], NULL));
		g_Client.GetStrafeData(slot).frame.StrafeToViewAngles = false;
	}
	else
	{
		g_Client.GetStrafeData(slot).frame.StrafeToViewAngles = true;
	}
}

CON_COMMAND(tas_strafe_buttons, "Sets the strafing buttons. The format is 4 digits together: \"<AirLeft><AirRight><GroundLeft><GroundRight>\". The default (auto-detect) is empty string: \"\".\nTable of buttons:\n\t0 - W\n\t1 - WA\n\t2 - A\n\t3 - SA\n\t4 - S\n\t5 - SD\n\t6 - D\n\t7 - WD")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();
	Strafe::StrafeData &strafeData = g_Client.GetStrafeData(slot);

	if (args.ArgC() < 2)
	{
		Msg("Value: %d%d%d%d | Usage: tas_strafe_buttons [buttons]\n", strafeData.frame.buttons.AirLeft,
																		strafeData.frame.buttons.AirRight,
																		strafeData.frame.buttons.GroundLeft,
																		strafeData.frame.buttons.GroundRight);
		return;
	}

	const char *pszButtons = args[1];
	int buttons = atoi(pszButtons);

	if (*pszButtons && buttons)
	{
		strafeData.frame.buttons.GroundRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		strafeData.frame.buttons.GroundLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		strafeData.frame.buttons.AirRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		strafeData.frame.buttons.AirLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;

		strafeData.frame.UseGivenButtons = true;
	}
	else
	{
		strafeData.frame.UseGivenButtons = false;
	}
}

CON_COMMAND(tas_strafe_vectorial, "Determines if strafing uses vectorial strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_vectorial [0/1]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.StrafeVectorial);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.StrafeVectorial = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_vectorial_increment, "Determines how fast the player yaw angle moves towards the target yaw angle. 0 for no movement, 180 for instant snapping. Has no effect on strafing speed")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_increment [value]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialIncrement);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialIncrement = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_vectorial_offset, "Determines the target view angle offset from tas_strafe_yaw")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_offset [value]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialOffset);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialOffset = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_vectorial_snap, "Determines when the yaw angle snaps to the target yaw. Mainly used to prevent ABHing from resetting the yaw angle to the back on every jump")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_snap [value]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialSnap);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.VectorialSnap = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_ignore_ground, "Strafe only in air")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_ignore_ground [0/1]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.IgnoreGround);
		return;
	}

	g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).frame.IgnoreGround = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_tickrate, "Sets the tickrate used in strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_tickrate [>= 10]\n", g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).vars.Frametime);
		return;
	}

	int tickrate = atoi(args[1]);

	if (tickrate >= 10)
		g_Client.GetStrafeData(GET_ACTIVE_SPLITSCREEN_SLOT()).vars.Frametime = 1.0f / tickrate;
}

// Input Manager (these commands also server-side when you use specified player index)

CON_COMMAND(tas_im_record, "Start recording player's inputs in a file")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: tas_im_record [filename] [optional: index]\n");
		return;
	}

	const char *pszFilename = args[1];

	if (args.ArgC() >= 3)
	{
		if (g_Server.IsInitialized())
			server__tas_im_record(pszFilename, atoi(args[2]));
	}
	else
	{
		QAngle va;
		float orientation[3][3];
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

		if (!g_Client.GetLocalPlayer(nSlot))
			return;

		g_pEngineClient->GetViewAngles(va);

		*reinterpret_cast<QAngle *>(orientation[1]) = va;
		*reinterpret_cast<Vector *>(orientation[0]) = *reinterpret_cast<Vector *>(GetOffset(g_Client.GetLocalPlayer(nSlot), RecvPropOffsets::m_vecOrigin));
		*reinterpret_cast<Vector *>(orientation[2]) = *reinterpret_cast<Vector *>(GetOffset(g_Client.GetLocalPlayer(nSlot), RecvPropOffsets::m_vecVelocity));

		g_InputManager.Record(pszFilename, g_InputManager.GetClientInputData(nSlot), orientation);
	}
}

CON_COMMAND(tas_im_play, "Playback player's inputs from a file")
{
	if (args.ArgC() < 2)
	{
		Msg("Usage: tas_im_play [filename] [optional: index]\n");
		return;
	}

	const char *pszFilename = args[1];

	if (args.ArgC() >= 3)
	{
		if (g_Server.IsInitialized())
			server__tas_im_play(pszFilename, atoi(args[2]));
	}
	else
	{
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
		InputData *pInputData = g_InputManager.GetClientInputData(nSlot);

		if (!g_Client.GetLocalPlayer(nSlot))
			return;

		g_InputManager.Playback(pszFilename, pInputData);

		// Using server-side code here
		if (pInputData->active && UTIL_PlayerByIndex && tas_im_tp.GetBool())
		{
			int nIndex = EntIndexOfBaseEntity((IClientEntity *)g_Client.GetLocalPlayer(nSlot));
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(nIndex);

			Teleport((CBaseEntity *)pPlayer,
						(Vector *)pInputData->baseInfo.origin,
						(QAngle *)pInputData->baseInfo.viewangles,
						(Vector *)pInputData->baseInfo.velocity);
		}
	}
}

CON_COMMAND(tas_im_split, "Split playback of player's inputs")
{
	if (args.ArgC() >= 2)
	{
		if (g_Server.IsInitialized())
			server__tas_im_split(atoi(args[1]));
	}
	else
	{
		g_InputManager.Split(g_InputManager.GetClientInputData(GET_ACTIVE_SPLITSCREEN_SLOT()));
	}
}

CON_COMMAND(tas_im_stop, "Stop record/playback player's inputs")
{
	if (args.ArgC() >= 2)
	{
		if (g_Server.IsInitialized())
			server__tas_im_stop(atoi(args[1]));
	}
	else
	{
		g_InputManager.Stop(g_InputManager.GetClientInputData(GET_ACTIVE_SPLITSCREEN_SLOT()));
	}
}

CON_COMMAND(tas_im_stop_all, "Stop record/playback inputs of all players")
{
	if (g_InputManager.GetClientInputData(0)->input)
		g_InputManager.Stop(g_InputManager.GetClientInputData(0));

	if (g_InputManager.GetClientInputData(1)->input)
		g_InputManager.Stop(g_InputManager.GetClientInputData(1));

	for (int i = 1; i < MAXCLIENTS; ++i)
	{
		if (g_InputManager.GetServerInputData(i)->input)
			g_InputManager.Stop(g_InputManager.GetServerInputData(i));
	}
}

// ConVars

ConVar wait_frames_pause("wait_frames_pause", "0", FCVAR_RELEASE, "Pause execution of wait_frames commands");

ConVar tas_setpitch("tas_setpitch", "0", FCVAR_RELEASE, "Set the Pitch angle", OnSetAngle);
ConVar tas_setyaw("tas_setyaw", "0", FCVAR_RELEASE, "Set the Yaw angle", OnSetAngle);
ConVar tas_setanglespeed("tas_setanglespeed", "360", FCVAR_RELEASE, "Speed of setting angles when using tas_setpitch/tas_setyaw");

ConVar ss_forceuser("ss_forceuser", "0", FCVAR_RELEASE, "Process all splitscreen players");