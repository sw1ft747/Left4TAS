// C++
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

#include "../structs/cl_splitscreen.h"
#include "game/shared/igamemovement.h"

#define SIZEOF_USERCMD 88
#define MULTIPLAYER_BACKUP 150

#ifdef strdup
#undef strdup
#endif

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class C_BaseEntity;
class C_BasePlayer;
class SplitPlayer_t;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern IBaseClientDLL *g_pClient;
extern IVEngineClient *g_pEngineClient;
extern IClientEntityList *g_pClientEntityList;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static bool s_bProcessClientMode = false;
static uint32_t s_nTicksOnGround = 0;
static int s_nActiveSlot = -1;
static int s_lockedSlot = -1;

int g_nForceUser = -1;
bool g_bInSplitScreen = false;

CUserCmd *pCmd = NULL;
ISplitScreen *splitscreen = NULL;

// class/interface *variable[MAX_SPLITSCREEN_PLAYERS];
IClientMode **g_pClientMode = NULL;
C_BasePlayer **s_pLocalPlayer = NULL;
SplitPlayer_t **m_SplitScreenPlayers = NULL;

bool g_bAutoJumpClient[MAX_SPLITSCREEN_PLAYERS] = { true, true };
Strafe::StrafeData g_StrafeData[MAX_SPLITSCREEN_PLAYERS];
input_state g_InputState[MAX_SPLITSCREEN_PLAYERS];

bhop_info g_bhopInfo;

std::vector<CWaitFrame *> g_vecWaitFrames;
std::vector<CWaitFrame *> g_vecWaitFrames_SS; // queue of commands for the 2nd splitscreen player
std::vector<CSimpleTrigger *> g_vecTriggers;

void OnConVarChange(IConVar *var, const char *pOldValue, float flOldValue);
void OnSetAngle(IConVar *var, const char *pOldValue, float flOldValue);

//-----------------------------------------------------------------------------
// Native cvars
//-----------------------------------------------------------------------------

ConVar *sv_airaccelerate;
ConVar *sv_accelerate;
ConVar *sv_friction;
ConVar *sv_maxspeed;
ConVar *sv_stopspeed;
ConVar *in_forceuser;
ConVar *cl_mouseenable;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

VTABLE_HOOK(ISplitScreen_Hook);
VTABLE_HOOK(IBaseClientDLL_Hook);

TRAMPOLINE_HOOK(CCSModeManager__Init_Hook);
TRAMPOLINE_HOOK(GetButtonBits_Hook);
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

GetButtonBitsFn GetButtonBits_Original = NULL;
CreateMoveFn CreateMove_Original = NULL;
ControllerMoveFn ControllerMove_Original = NULL;
AdjustAnglesFn AdjustAngles_Original = NULL;

CheckJumpButtonClientFn CheckJumpButtonClient_Original = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CWaitFrame::CWaitFrame(long long nFrames, const char *szCommand)
{
	m_nFrames = nFrames;
	m_szCommand = strdup(szCommand);
}

CWaitFrame::~CWaitFrame()
{
	free((void *)m_szCommand);
}

//-----------------------------------------------------------------------------

inline void *GetLocalClient(int nSlot)
{
	return GetOffset(m_SplitScreenPlayers[nSlot], 8);
}

void GetViewAngles(int nSlot, QAngle &va)
{
	void *client = GetLocalClient(nSlot);
	va = *reinterpret_cast<QAngle *>(GetOffset(client, Offsets::Variables::m_Client__viewangles));
}

void SetViewAngles(int nSlot, QAngle &va)
{
	void *client = GetLocalClient(nSlot);

	if (va.IsValid())
	{
		*reinterpret_cast<QAngle *>(GetOffset(client, Offsets::Variables::m_Client__viewangles)) = va;
	}
	else
	{
		Warning("CEngineClient::SetViewAngles:  rejecting invalid value [%f %f %f]\n", VectorExpand(va));
		*reinterpret_cast<QAngle *>(GetOffset(client, Offsets::Variables::m_Client__viewangles)) = vec3_angle;
	}
}

//-----------------------------------------------------------------------------

inline void ExecuteClientCmd(int nSlot, const char *pszCommand)
{
	Cbuf_AddText(static_cast<ECommandTarget_t>(nSlot), pszCommand, kCommandSrcCode);
	Cbuf_Execute();
}

inline void ExecuteClientCmd(ECommandTarget_t target, const char *pszCommand)
{
	Cbuf_AddText(target, pszCommand, kCommandSrcCode);
	Cbuf_Execute();
}

//-----------------------------------------------------------------------------

void UpdateStrafeData(int nSlot)
{
	C_BasePlayer *pLocal = s_pLocalPlayer[nSlot];

	*reinterpret_cast<Vector *>(g_StrafeData[nSlot].player.Velocity) = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecVelocity));
	*reinterpret_cast<Vector *>(g_StrafeData[nSlot].player.Origin) = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecOrigin));

	g_StrafeData[nSlot].vars.OnGround = GetGroundEntity(pLocal) != NULL;
	g_StrafeData[nSlot].vars.EntFriction = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flFriction));
	g_StrafeData[nSlot].vars.ReduceWishspeed = g_StrafeData[nSlot].vars.OnGround && (*reinterpret_cast<int *>(GetOffset(pLocal, RecvPropOffsets::m_fFlags)) & FL_DUCKING);

	g_StrafeData[nSlot].vars.Maxspeed = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flMaxspeed));
	g_StrafeData[nSlot].vars.Stopspeed = sv_stopspeed->GetFloat();
	g_StrafeData[nSlot].vars.Friction = sv_friction->GetFloat();

	g_StrafeData[nSlot].vars.Accelerate = sv_accelerate->GetFloat();
	g_StrafeData[nSlot].vars.Airaccelerate = sv_airaccelerate->GetFloat();
}

//-----------------------------------------------------------------------------

bool ChangeAngle(float &angle, float target)
{
	float normalizedDiff = Strafe::NormalizeDeg(static_cast<double>(target) - angle);
	if (std::abs(normalizedDiff) > tas_setanglespeed.GetFloat())
	{
		angle += std::copysign(tas_setanglespeed.GetFloat(), normalizedDiff);
		return true;
	}
	else
	{
		angle = target;
		return false;
	}
}

bool ChangeAngleBySpeed(float &angle, float target, float speed)
{
	float normalizedDiff = Strafe::NormalizeDeg(static_cast<double>(target) - angle);
	if (std::abs(normalizedDiff) > speed)
	{
		angle += std::copysign(speed, normalizedDiff);
		return true;
	}
	else
	{
		angle = target;
		return false;
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

bool __fastcall AddSplitScreenUser_Hooked(void *thisptr, int edx, int nSlot, int nPlayerIndex)
{
	bool result = AddSplitScreenUser_Original(thisptr, nSlot, nPlayerIndex);

	if (result)
		g_bInSplitScreen = true;

	return result;
}

bool __fastcall RemoveSplitScreenUser_Hooked(void *thisptr, int edx, int nSlot, int nPlayerIndex)
{
	bool result = RemoveSplitScreenUser_Original(thisptr, nSlot, nPlayerIndex);

	if (result)
		g_bInSplitScreen = false;

	return result;
}

void __stdcall HudUpdate_Hooked(bool bActive)
{
	HudUpdate_Original(bActive);

	Vector *vecOrigin = NULL;
	Vector *vecOrigin_SS = NULL;

	if (s_pLocalPlayer[0])
		vecOrigin = reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[0], RecvPropOffsets::m_vecOrigin));

	if (s_pLocalPlayer[1])
		vecOrigin_SS = reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[1], RecvPropOffsets::m_vecOrigin));

	for (size_t i = 0; i < g_vecTriggers.size(); ++i)
	{
		CSimpleTrigger *trigger = g_vecTriggers[i];

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

		g_vecTriggers.erase(g_vecTriggers.begin() + i);
		--i;
	}
}

int __fastcall GetButtonBits_Hooked(void *thisptr, int edx, bool bResetState)
{
	int buttons = GetButtonBits_Original(thisptr, bResetState);

	//if (bResetState == 1)
	//{
	//	const int IN_JUMP = 1 << 1;

	//	if (g_InputState.bForceJump)
	//	{
	//		g_InputState.bForceJump = false;
	//		buttons |= IN_JUMP;
	//	}
	//}

	return buttons;
}

bool __fastcall IClientMode__CreateMove_Hooked(void *thisptr, int edx, float flInputSampleTime, CUserCmd *cmd)
{
	bool overridden = false;
	bool result = IClientMode__CreateMove_Original(thisptr, flInputSampleTime, cmd);

	if (s_bProcessClientMode && g_InputDataClient[s_nActiveSlot].input)
	{
		if (g_InputDataClient[s_nActiveSlot].recording)
		{
			g_InputManager.SaveInput(&g_InputDataClient[s_nActiveSlot], cmd, true);
		}
		else
		{
			g_InputManager.ReadInput(&g_InputDataClient[s_nActiveSlot], cmd, s_pLocalPlayer[s_nActiveSlot], true);
			overridden = true;
		}

		++g_InputDataClient[s_nActiveSlot].frames;
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

	g_nForceUser = in_forceuser->GetInt();
	s_nActiveSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
	s_bProcessClientMode = true;

	void *pPerUser = &reinterpret_cast<DWORD *>(thisptr)[0x2F * s_nActiveSlot + 13];
	DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(pPerUser, Offsets::Variables::m_pCommands));

	pCmd = reinterpret_cast<CUserCmd *>(m_pCommands + SIZEOF_USERCMD * (sequence_number % MULTIPLAYER_BACKUP));

	if (ss_forceuser.GetBool())
		in_forceuser->SetValue(s_nActiveSlot);

	CreateMove_Original(thisptr, sequence_number, input_sample_frametime, active);

	in_forceuser->SetValue(g_nForceUser);
	pCmd = NULL;

	// wait_frames stuff
	if (wait_frames_pause.GetBool())
		return;

	if (!s_nActiveSlot)
	{
		for (size_t i = 0; i < g_vecWaitFrames.size(); ++i)
		{
			CWaitFrame *frame_cmd = g_vecWaitFrames[i];

			if (--(frame_cmd->m_nFrames) <= 0)
			{
				ExecuteClientCmd(s_nActiveSlot, frame_cmd->m_szCommand);
				delete frame_cmd;

				g_vecWaitFrames.erase(g_vecWaitFrames.begin() + i);
				--i;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < g_vecWaitFrames_SS.size(); ++i)
		{
			CWaitFrame *frame_cmd = g_vecWaitFrames_SS[i];

			if (--(frame_cmd->m_nFrames) <= 0)
			{
				ExecuteClientCmd(s_nActiveSlot, frame_cmd->m_szCommand);
				delete frame_cmd;

				g_vecWaitFrames_SS.erase(g_vecWaitFrames_SS.begin() + i);
				--i;
			}
		}
	}
}

void __fastcall ControllerMove_Hooked(void *thisptr, int edx, int nSlot, float frametime, CUserCmd *cmd)
{
	int nEnableMouse = cl_mouseenable->GetInt();

	// Don't let a player control the camera angles of another player..
	if (ss_forceuser.GetBool() && g_nForceUser != nSlot)
		cl_mouseenable->SetValue(0);

	ControllerMove_Original(thisptr, nSlot, frametime, cmd);

	cl_mouseenable->SetValue(nEnableMouse);
}

void __stdcall AdjustAngles_Hooked(int nSlot, float frametime)
{
	AdjustAngles_Original(nSlot, frametime);

	if (!pCmd)
		return;

	QAngle viewangles;
	GetViewAngles(nSlot, viewangles);

	bool bYawChanged = false;
	bool bOnGround = false;

	if (!g_InputDataClient[nSlot].input)
	{
		if (g_InputState[nSlot].bSetAngles)
		{
			bool pitchChanged = ChangeAngleBySpeed(viewangles[PITCH], g_InputState[nSlot].setAngles[PITCH], g_InputState[nSlot].setAnglesSpeed[PITCH]);
			bool yawChanged = bYawChanged = ChangeAngleBySpeed(viewangles[YAW], g_InputState[nSlot].setAngles[YAW], g_InputState[nSlot].setAnglesSpeed[YAW]);

			if (!pitchChanged && !yawChanged)
				g_InputState[nSlot].bSetAngles = false;
		}
		else
		{
			if (g_InputState[nSlot].bSetPitch)
			{
				g_InputState[nSlot].bSetPitch = ChangeAngle(viewangles[PITCH], tas_setpitch.GetFloat());
			}

			if (g_InputState[nSlot].bSetYaw)
			{
				g_InputState[nSlot].bSetYaw = bYawChanged = ChangeAngle(viewangles[YAW], tas_setyaw.GetFloat());
			}
		}
	}

	if (!g_InputDataClient[nSlot].input && g_StrafeData[nSlot].frame.Strafe)
	{
		UpdateStrafeData(nSlot);

		if (g_nForceUser == nSlot && g_StrafeData[nSlot].vars.OnGround)
			++s_nTicksOnGround;

		if (!(g_StrafeData[nSlot].frame.IgnoreGround && g_StrafeData[nSlot].vars.OnGround))
		{
			Strafe::ProcessedFrame out;
			out.Yaw = viewangles[YAW];

			if (g_StrafeData[nSlot].frame.StrafeToViewAngles)
			{
				if (g_StrafeData[nSlot].frame.StrafeVectorial)
					g_StrafeData[nSlot].frame.SetYaw(viewangles[YAW]);
				else
					g_StrafeData[nSlot].frame.SetYaw(0.0);
			}

			Strafe::Friction(g_StrafeData[nSlot]);

			if (g_StrafeData[nSlot].frame.StrafeVectorial)
				Strafe::StrafeVectorial(g_StrafeData[nSlot], out, bYawChanged);
			else if (!bYawChanged)
				Strafe::Strafe(g_StrafeData[nSlot], out);

			if (out.Processed)
			{
				pCmd->forwardmove = out.Forwardspeed;
				pCmd->sidemove = out.Sidespeed;

				viewangles[YAW] = static_cast<float>(out.Yaw);
			}
		}
	}
	else if (g_nForceUser == nSlot && GetGroundEntity(s_pLocalPlayer[nSlot]))
	{
		++s_nTicksOnGround;
	}

	SetViewAngles(nSlot, viewangles);
}

// Called multiple times when you have ping, this game sucks
bool __fastcall CheckJumpButtonClient_Hooked(void *thisptr, int edx)
{
	int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

	CMoveData *mv = reinterpret_cast<CMoveData *>(((uintptr_t *)thisptr)[2]);
	const int IN_JUMP = mv->m_nOldButtons & (1 << 1);

	if (tas_autojump.GetBool() && g_bAutoJumpClient[nSlot])
		mv->m_nOldButtons &= ~IN_JUMP;

	bool bJumped = CheckJumpButtonClient_Original(thisptr);

	if (bJumped)
	{
		int nForceUser = in_forceuser->GetInt();

		if (nSlot == nForceUser)
		{
			float flCurrentSpeed = reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[nForceUser], RecvPropOffsets::m_vecVelocity))->Length2D();

			if (s_nTicksOnGround <= 3)
			{
				float flLoss = g_bhopInfo.flLastSpeed - flCurrentSpeed;

				g_bhopInfo.flSpeedLoss = flLoss > 0.0f ? flLoss : 0.0f;
				g_bhopInfo.flPercentage = (flCurrentSpeed / g_bhopInfo.flLastSpeed) * 100.0f;

				if (g_bhopInfo.flPercentage > 1000000.0f)
					g_bhopInfo.flPercentage = 100.0f;

				++g_bhopInfo.nJumps;
			}
			else
			{
				g_bhopInfo.nJumps = 0;
				g_bhopInfo.flSpeedLoss = 0.0f;
				g_bhopInfo.flPercentage = 100.0f;
			}

			g_bhopInfo.flLastSpeed = flCurrentSpeed;
			s_nTicksOnGround = 0;
		}
	}

	mv->m_nOldButtons |= IN_JUMP;
	return bJumped;
}

void __fastcall CCSModeManager__Init_Hooked(void *thisptr, int edx)
{
	CCSModeManager__Init_Original(thisptr);

	if (IClientMode__CreateMove_Original)
		return;

	IClientMode__CreateMove_Original = (IClientMode__CreateMoveFn)CVTableHook::HookFunction(g_pClientMode[0], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
	CVTableHook::HookFunction(g_pClientMode[1], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
}

//-----------------------------------------------------------------------------
// Init/release client-side module
//-----------------------------------------------------------------------------

bool IsClientModuleInit()
{
	return __INITIALIZED__;
}

bool InitClientModule()
{
	memset(&g_InputDataClient, 0, sizeof(g_InputDataClient));

	INSTRUCTION instruction;

	if (!IsEngineModuleInit())
		return false;

	// s_pLocalPlayer
	void *pCheckForLocalPlayer = FIND_PATTERN(L"client.dll", Patterns::Client::C_BasePlayer__CheckForLocalPlayer);

	if (!pCheckForLocalPlayer)
	{
		FailedInit("C_BasePlayer::CheckForLocalPlayer");
		return false;
	}

	//void *pGetButtonBits = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__GetButtonBits);

	//if (!pGetButtonBits)
	//{
	//	FailedInit("CInput::GetButtonBits");
	//	return false;
	//}
	
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

	if (!IsServerModuleInit())
	{
		g_bAutoJumpClient[0] = false;
		g_bAutoJumpClient[1] = false;
	}

	// IClientMode *g_pClientMode[MAX_SPLITSCREEN_PLAYERS];
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
			g_pClientMode = reinterpret_cast<IClientMode **>(instruction.op2.displacement);
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
	
	// SplitPlayer_t *m_SplitScreenPlayers[MAX_SPLITSCREEN_PLAYERS];
	get_instruction(&instruction, (BYTE *)g_pGetBaseLocalClient, MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		m_SplitScreenPlayers = reinterpret_cast<SplitPlayer_t **>(instruction.op2.displacement);
	}
	else
	{
		FailedInit("m_SplitScreenPlayers");
		return false;
	}

	// C_BasePlayer *s_pLocalPlayer[MAX_SPLITSCREEN_PLAYERS];
	get_instruction(&instruction, (BYTE *)GetOffset(pCheckForLocalPlayer, Offsets::Variables::s_pLocalPlayer), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		s_pLocalPlayer = reinterpret_cast<C_BasePlayer **>(instruction.op2.displacement);
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

	g_nForceUser = in_forceuser->GetInt();

	// IClientMode hooks
	if (g_pClientMode[0] && g_pClientMode[1])
	{
		IClientMode__CreateMove_Original = (IClientMode__CreateMoveFn)CVTableHook::HookFunction(g_pClientMode[0], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
		CVTableHook::HookFunction(g_pClientMode[1], IClientMode__CreateMove_Hooked, Offsets::Functions::IClientMode__CreateMove);
	}

	// Trampoline hook
	HOOK_FUNCTION(CCSModeManager__Init_Hook, pCCSModeManager__Init, CCSModeManager__Init_Hooked, CCSModeManager__Init_Original, CCSModeManager__InitFn);
	//HOOK_FUNCTION(GetButtonBits_Hook, pGetButtonBits, GetButtonBits_Hooked, GetButtonBits_Original, GetButtonBitsFn);
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

	__INITIALIZED__ = true;
	return true;
}

void ReleaseClientModule()
{
	if (!__INITIALIZED__)
		return;

	if (IClientMode__CreateMove_Original && g_pClientMode[0] && g_pClientMode[1])
	{
		CVTableHook::UnhookFunction(g_pClientMode[0], IClientMode__CreateMove_Original, Offsets::Functions::IClientMode__CreateMove);
		CVTableHook::UnhookFunction(g_pClientMode[1], IClientMode__CreateMove_Original, Offsets::Functions::IClientMode__CreateMove);
	}

	// Unhook functions
	UNHOOK_FUNCTION(CCSModeManager__Init_Hook);
	//UNHOOK_FUNCTION(GetButtonBits_Hook);
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
		SET_ACTIVE_SPLIT_SCREEN_PLAYER_SLOT(nSlot);
		s_lockedSlot = nSlot;
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

	ExecuteClientCmd(CBUF_FIRST_PLAYER, map_buffer);
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
		ExecuteClientCmd(nSlot, args.Arg(1));
}

CON_COMMAND(autojump, "Toggle autojump for local player")
{
	if (IsServerModuleInit())
	{
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();
		int nPlayerIndex = g_pEngineClient->GetLocalPlayer();

		g_bAutoJumpClient[nSlot] = !g_bAutoJumpClient[nSlot];
		g_bAutoJumpServer[nPlayerIndex] = !g_bAutoJumpServer[nPlayerIndex];
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
				ExecuteClientCmd(0, pszCommand);
			else if (IS_VALID_SPLIT_SCREEN_SLOT(1))
				ExecuteClientCmd(1, pszCommand);
		}
		else
		{
			CWaitFrame *frame_cmd = new CWaitFrame(nFrames, pszCommand);

			if (nSlot == 0 && args.ArgC() == 3)
				g_vecWaitFrames.push_back(frame_cmd);
			else
				g_vecWaitFrames_SS.push_back(frame_cmd);
		}
	}
}

CON_COMMAND(wait_frames_clear_queue, "Clear the queue of wait_frames")
{
	if (args.ArgC() == 1)
	{
		for (size_t i = 0; i < g_vecWaitFrames.size(); ++i)
			delete g_vecWaitFrames[i];

		for (size_t i = 0; i < g_vecWaitFrames_SS.size(); ++i)
			delete g_vecWaitFrames_SS[i];

		g_vecWaitFrames.clear();
		g_vecWaitFrames_SS.clear();
	}
	else if (args.ArgC() >= 2)
	{
		if (atoi(args.Arg(1)) == 0)
		{
			for (size_t i = 0; i < g_vecWaitFrames.size(); ++i)
				delete g_vecWaitFrames[i];

			g_vecWaitFrames.clear();
		}
		else
		{
			for (size_t i = 0; i < g_vecWaitFrames_SS.size(); ++i)
				delete g_vecWaitFrames_SS[i];

			g_vecWaitFrames_SS.clear();
		}
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

	CSimpleTrigger *trigger = new CSimpleTrigger(args[1], vecStart, vecEnd);
	trigger->SetCustomData(mode);

	g_vecTriggers.push_back(trigger);
}

CON_COMMAND(tas_remove_triggers, "Remove all triggers")
{
	if (args.ArgC() == 1)
	{
		for (size_t i = 0; i < g_vecTriggers.size(); ++i)
			delete g_vecTriggers[i];

		g_vecTriggers.clear();
	}
	else if (args.ArgC() >= 2)
	{
		if (atoi(args.Arg(1)) == 0)
		{
			for (size_t i = 0; i < g_vecTriggers.size(); ++i)
			{
				if (g_vecTriggers[i]->GetCustomData() == 1)
				{
					delete g_vecTriggers[i];
					g_vecTriggers.erase(g_vecTriggers.begin() + i);
					--i;
				}
			}
		}
		else
		{
			for (size_t i = 0; i < g_vecTriggers.size(); ++i)
			{
				if (g_vecTriggers[i]->GetCustomData() == 2)
				{
					delete g_vecTriggers[i];
					g_vecTriggers.erase(g_vecTriggers.begin() + i);
					--i;
				}
			}
		}
	}
}

CON_COMMAND(tas_reset_angles_change, "Reset change of angles")
{
	for (int i = 0; i < MAX_SPLITSCREEN_PLAYERS; ++i)
	{
		g_InputState[i].bSetAngles = false;
		g_InputState[i].bSetPitch = false;
		g_InputState[i].bSetYaw = false;
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
		GetViewAngles(nSlot, viewangles);

		float flPitch = atof(args.Arg(1));
		float flYaw = atof(args.Arg(2));

		float flNormalizedPitch = Strafe::NormalizeDeg(static_cast<double>(flPitch) - viewangles[PITCH]);
		float flNormalizedYaw = Strafe::NormalizeDeg(static_cast<double>(flYaw) - viewangles[YAW]);

		g_InputState[nSlot].setAngles[PITCH] = flPitch;
		g_InputState[nSlot].setAngles[YAW] = flYaw;

		g_InputState[nSlot].setAnglesSpeed[PITCH] = std::abs(flNormalizedPitch) / nFrames;
		g_InputState[nSlot].setAnglesSpeed[YAW] = std::abs(flNormalizedYaw) / nFrames;

		g_InputState[nSlot].bSetAngles = true;
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
	GetViewAngles(nSlot, viewangles);

	viewangles[PITCH] = strtof(args[1], NULL);

	SetViewAngles(nSlot, viewangles);
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
	GetViewAngles(nSlot, viewangles);

	viewangles[YAW] = strtof(args[1], NULL);

	SetViewAngles(nSlot, viewangles);
}

// Strafing Tools

CON_COMMAND(tas_strafe, "Enable TAS strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe [0/1]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.Strafe);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.Strafe = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_dir, "TAS strafe directions:\n\t0 - to the left\n\t1 - to the right\n\t2 - best strafe\n\t3 - to the yaw given in tas_strafe_yaw\n\t4 - to the point given in tas_strafe_point")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_dir [0-4]\n", static_cast<int>(g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.GetDir()));
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.SetDir(static_cast<Strafe::StrafeDir>(atoi(args[1])));
}

CON_COMMAND(tas_strafe_type, "TAS strafe types:\n\t0 - Max acceleration strafing\n\t1 - Max angle strafing\n\t2 - Max deceleration strafing\n\t3 - Const speed strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_type [0-3]\n", static_cast<int>(g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.GetType()));
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.SetType(static_cast<Strafe::StrafeType>(atoi(args[1])));
}

CON_COMMAND(tas_strafe_point, "2D point to strafe with tas_strafe_dir = 4")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();

	if (args.ArgC() < 3)
	{
		Msg("Value: { %f, %f } | Usage: tas_strafe_point [x] [y]\n", g_StrafeData[slot].frame.GetX(), g_StrafeData[slot].frame.GetY());
		return;
	}

	g_StrafeData[slot].frame.SetX(strtof(args[1], NULL));
	g_StrafeData[slot].frame.SetY(strtof(args[2], NULL));
}

CON_COMMAND(tas_strafe_yaw, "Yaw to strafe to with tas_strafe_dir = 3")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();

	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_yaw [yaw]\n", g_StrafeData[slot].frame.StrafeToViewAngles ? 0.0f : g_StrafeData[slot].frame.GetYaw());
		return;
	}

	const char *pszYaw = args[1];

	if (*pszYaw)
	{
		g_StrafeData[slot].frame.SetYaw(strtof(args[1], NULL));
		g_StrafeData[slot].frame.StrafeToViewAngles = false;
	}
	else
	{
		g_StrafeData[slot].frame.StrafeToViewAngles = true;
	}
}

CON_COMMAND(tas_strafe_buttons, "Sets the strafing buttons. The format is 4 digits together: \"<AirLeft><AirRight><GroundLeft><GroundRight>\". The default (auto-detect) is empty string: \"\".\nTable of buttons:\n\t0 - W\n\t1 - WA\n\t2 - A\n\t3 - SA\n\t4 - S\n\t5 - SD\n\t6 - D\n\t7 - WD")
{
	int slot = GET_ACTIVE_SPLITSCREEN_SLOT();

	if (args.ArgC() < 2)
	{
		Msg("Value: %d%d%d%d | Usage: tas_strafe_buttons [buttons]\n", g_StrafeData[slot].frame.buttons.AirLeft,
																		g_StrafeData[slot].frame.buttons.AirRight,
																		g_StrafeData[slot].frame.buttons.GroundLeft,
																		g_StrafeData[slot].frame.buttons.GroundRight);
		return;
	}

	const char *pszButtons = args[1];
	int buttons = atoi(pszButtons);

	if (*pszButtons && buttons)
	{
		g_StrafeData[slot].frame.buttons.GroundRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		g_StrafeData[slot].frame.buttons.GroundLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		g_StrafeData[slot].frame.buttons.AirRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
		g_StrafeData[slot].frame.buttons.AirLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;

		g_StrafeData[slot].frame.UseGivenButtons = true;
	}
	else
	{
		g_StrafeData[slot].frame.UseGivenButtons = false;
	}
}

CON_COMMAND(tas_strafe_vectorial, "Determines if strafing uses vectorial strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_vectorial [0/1]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.StrafeVectorial);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.StrafeVectorial = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_vectorial_increment, "Determines how fast the player yaw angle moves towards the target yaw angle. 0 for no movement, 180 for instant snapping. Has no effect on strafing speed")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_increment [value]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialIncrement);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialIncrement = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_vectorial_offset, "Determines the target view angle offset from tas_strafe_yaw")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_offset [value]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialOffset);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialOffset = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_vectorial_snap, "Determines when the yaw angle snaps to the target yaw. Mainly used to prevent ABHing from resetting the yaw angle to the back on every jump")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_vectorial_snap [value]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialSnap);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.VectorialSnap = strtof(args[1], NULL);
}

CON_COMMAND(tas_strafe_ignore_ground, "Strafe only in air")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %d | Usage: tas_strafe_ignore_ground [0/1]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.IgnoreGround);
		return;
	}

	g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].frame.IgnoreGround = (atoi(args[1]) == 0 ? false : true);
}

CON_COMMAND(tas_strafe_tickrate, "Sets the tickrate used in strafing")
{
	if (args.ArgC() < 2)
	{
		Msg("Value: %f | Usage: tas_strafe_tickrate [>= 10]\n", g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].vars.Frametime);
		return;
	}

	int tickrate = atoi(args[1]);

	if (tickrate >= 10)
		g_StrafeData[GET_ACTIVE_SPLITSCREEN_SLOT()].vars.Frametime = 1.0f / tickrate;
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
		if (IsServerModuleInit())
			server__tas_im_record(pszFilename, atoi(args[2]));
	}
	else
	{
		QAngle va;
		float orientation[3][3];
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

		if (!s_pLocalPlayer[nSlot])
			return;

		GetViewAngles(nSlot, va);

		*reinterpret_cast<QAngle *>(orientation[1]) = va;
		*reinterpret_cast<Vector *>(orientation[0]) = *reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[nSlot], RecvPropOffsets::m_vecOrigin));
		*reinterpret_cast<Vector *>(orientation[2]) = *reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[nSlot], RecvPropOffsets::m_vecVelocity));

		g_InputManager.Record(pszFilename, &g_InputDataClient[nSlot], orientation);
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
		if (IsServerModuleInit())
			server__tas_im_play(pszFilename, atoi(args[2]));
	}
	else
	{
		int nSlot = GET_ACTIVE_SPLITSCREEN_SLOT();

		if (!s_pLocalPlayer[nSlot])
			return;

		g_InputManager.Playback(pszFilename, &g_InputDataClient[nSlot]);

		// Uhh, using server-side code here
		if (g_InputDataClient[nSlot].active && __UTIL_PlayerByIndex && tas_im_tp.GetBool())
		{
			int nIndex = EntIndexOfBaseEntity((IClientEntity *)s_pLocalPlayer[nSlot]);
			CBasePlayer *pPlayer = UTIL_PlayerByIndex(nIndex);

			Teleport((CBaseEntity *)pPlayer,
						(Vector *)g_InputDataClient[nSlot].baseInfo.origin,
						(QAngle *)g_InputDataClient[nSlot].baseInfo.viewangles,
						(Vector *)g_InputDataClient[nSlot].baseInfo.velocity);
		}
	}
}

CON_COMMAND(tas_im_split, "Split playback of player's inputs")
{
	if (args.ArgC() >= 2)
	{
		if (IsServerModuleInit())
			server__tas_im_split(atoi(args[1]));
	}
	else
	{
		g_InputManager.Split(&g_InputDataClient[GET_ACTIVE_SPLITSCREEN_SLOT()]);
	}
}

CON_COMMAND(tas_im_stop, "Stop record/playback player's inputs")
{
	if (args.ArgC() >= 2)
	{
		if (IsServerModuleInit())
			server__tas_im_stop(atoi(args[1]));
	}
	else
	{
		g_InputManager.Stop(&g_InputDataClient[GET_ACTIVE_SPLITSCREEN_SLOT()]);
	}
}

CON_COMMAND(tas_im_stop_all, "Stop record/playback inputs of all players")
{
	if (g_InputDataClient[0].input)
		g_InputManager.Stop(&g_InputDataClient[0]);

	if (g_InputDataClient[1].input)
		g_InputManager.Stop(&g_InputDataClient[1]);

	for (int i = 1; i < MAXCLIENTS; ++i)
	{
		if (g_InputDataServer[i].input)
			g_InputManager.Stop(&g_InputDataServer[i]);
	}
}

// ConVars

ConVar wait_frames_pause("wait_frames_pause", "0", FCVAR_RELEASE, "Pause execution of wait_frames commands");

ConVar tas_setpitch("tas_setpitch", "0", FCVAR_RELEASE, "Set the Pitch angle", OnSetAngle);

ConVar tas_setyaw("tas_setyaw", "0", FCVAR_RELEASE, "Set the Yaw angle", OnSetAngle);

ConVar tas_setanglespeed("tas_setanglespeed", "360", FCVAR_RELEASE, "Speed of setting angles when using tas_setpitch/tas_setyaw");

ConVar ss_forceuser("ss_forceuser", "0", FCVAR_RELEASE, "Process all splitscreen players");