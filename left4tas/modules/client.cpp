// C++
// Client Module

#define SIZEOF_USERCMD 88
#define MULTIPLAYER_BACKUP 150

#include "client.h"

#include "../cvars.h"
#include "../patterns.h"
#include "../prop_offsets.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "vtable_hook.h"
#include "utils.h"
#include "strafe_utils.h"
#include "usercmd.h"

#include "../tools/recvpropmanager.h"
#include "../tools/misc.h"
#include "../tools/strafestuff.h"
#include "../tools/simple_trigger.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class C_BaseEntity;
class C_BasePlayer;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern ICvar *g_pCVar;
extern IBaseClientDLL *g_pClient;
extern IVEngineClient *g_pEngineClient;

extern void *g_pCheckJumpButtonClient;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static uint32_t nTicksOnGround = 0;

CUserCmd *pCmd = NULL;
C_BasePlayer **s_pLocalPlayer = NULL; // C_BasePlayer *s_pLocalPlayer[MAX_SPLITSCREEN_PLAYERS]; \ splitscreen support? ehe

std::vector<CWaitFrame *> g_vecWaitFrames;
std::vector<CSimpleTrigger *> g_vecTriggers;

input_state g_InputState;
bhop_info g_bhopInfo;

//-----------------------------------------------------------------------------
// Native cvars
//-----------------------------------------------------------------------------

ConVar *sv_airaccelerate;
ConVar *sv_accelerate;
ConVar *sv_friction;
ConVar *sv_maxspeed;
ConVar *sv_stopspeed;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

VTABLE_HOOK(IBaseClientDLL_Hook);
TRAMPOLINE_HOOK(GetButtonBits_Hook);
TRAMPOLINE_HOOK(CreateMove_Hook);
TRAMPOLINE_HOOK(AdjustAngles_Hook);
TRAMPOLINE_HOOK(CheckJumpButton_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

GetGroundEntityFn GetGroundEntity = NULL;

HudUpdateFn HudUpdate_Original = NULL;
GetButtonBitsFn GetButtonBits_Original = NULL;
CreateMoveFn CreateMove_Original = NULL;
AdjustAnglesFn AdjustAngles_Original = NULL;
CheckJumpButtonFn CheckJumpButton_Original = NULL;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

CWaitFrame::CWaitFrame(long long nFrames, const char *szCommand, size_t length)
{
	m_nFrames = nFrames;
	m_szCommand = new char[length + 1];
	m_szCommand[length] = 0;

	memcpy(m_szCommand, szCommand, length);
}

CWaitFrame::~CWaitFrame()
{
	delete[] m_szCommand;
}

bool DoAngleChange(float &angle, float target)
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

bool DoAngleChange2(float &angle, float target, float speed)
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

Strafe::PlayerData GetPlayerData()
{
	Strafe::PlayerData data;

	float *vel = reinterpret_cast<float *>(GetOffset(s_pLocalPlayer[0], RecvPropOffsets::m_vecVelocity));
	data.Velocity = Vector(vel[0], vel[1], vel[2]);

	return data;
}

Strafe::MovementVars GetMovementVars()
{
	C_BasePlayer *pLocal = s_pLocalPlayer[0];
	Strafe::MovementVars vars = Strafe::MovementVars();

	vars.OnGround = GetGroundEntity(pLocal) != NULL;
	vars.EntFriction = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flFriction));
	vars.ReduceWishspeed = vars.OnGround && (*reinterpret_cast<int *>(GetOffset(pLocal, RecvPropOffsets::m_fFlags)) & FL_DUCKING);

	vars.Accelerate = sv_accelerate->GetFloat();
	vars.Airaccelerate = sv_airaccelerate->GetFloat();

	vars.Maxspeed = *reinterpret_cast<float *>(GetOffset(pLocal, RecvPropOffsets::m_flMaxspeed));
	vars.Stopspeed = sv_stopspeed->GetFloat();
	vars.Friction = sv_friction->GetFloat();

	if (*tas_strafe_tickrate.GetString() != '\0')
		vars.Frametime = 1.0f / tas_strafe_tickrate.GetInt();
	else
		vars.Frametime = 1.0f / 30.0f;

	if (*tas_force_wishspeed_cap.GetString() != '\0')
		vars.WishspeedCap = tas_force_wishspeed_cap.GetFloat();
	else
		vars.WishspeedCap = 30;

	return vars;
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void __stdcall HudUpdate_Hooked(bool bActive)
{
	HudUpdate_Original(bActive);

	// Triggers
	for (size_t i = 0; i < g_vecTriggers.size(); ++i)
	{
		CSimpleTrigger *trigger = g_vecTriggers[i];
		C_BasePlayer *pLocal = s_pLocalPlayer[0];

		if (pLocal)
		{
			Vector *vecOrigin = reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecOrigin));

			if (trigger->Think(vecOrigin))
			{
				delete trigger;

				g_vecTriggers.erase(g_vecTriggers.begin() + i);
				--i;
			}
		}
	}
}

int __fastcall GetButtonBits_Hooked(void *thisptr, int edx, bool bResetState)
{
	int buttons = GetButtonBits_Original(thisptr, bResetState);

	if (bResetState == 1)
	{
		const int IN_JUMP = 1 << 1;

		if (g_InputState.bForceJump)
		{
			g_InputState.bForceJump = false;
			buttons |= IN_JUMP;
		}
	}

	return buttons;
}

void __fastcall CreateMove_Hooked(void *thisptr, int edx, int sequence_number, float input_sample_frametime, bool active)
{
	// void *pPerUser = &(reinterpret_cast<DWORD *>(thisptr)[13]); // this[0x2F * nSlot + 13]
	// DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(pPerUser, Offsets::Variables::m_pCommands = 168));
	// 168 + 13 * 4 = 220 = 0xDC

	DWORD m_pCommands = *reinterpret_cast<DWORD *>(GetOffset(thisptr, Offsets::Variables::m_pCommands));
	pCmd = reinterpret_cast<CUserCmd *>(m_pCommands + SIZEOF_USERCMD * (sequence_number % MULTIPLAYER_BACKUP));

	CreateMove_Original(thisptr, sequence_number, input_sample_frametime, active);

	pCmd = NULL;

	// wait_frames stuff
	extern bool g_bGamePaused;

	if (wait_frames_pause.GetBool() || g_bGamePaused)
		return;

	for (size_t i = 0; i < g_vecWaitFrames.size(); ++i)
	{
		CWaitFrame *frame_cmd = g_vecWaitFrames[i];

		if (--(frame_cmd->m_nFrames) <= 0)
		{
			// void IVEngineClient::ExecuteClientCmd( const char *szCmdString ); \ ecx is not used
			GetVTableFunction<void(__stdcall *)(const char *)>(g_pEngineClient, Offsets::Functions::IVEngineClient__ExecuteClientCmd)(frame_cmd->m_szCommand);
			delete frame_cmd;

			g_vecWaitFrames.erase(g_vecWaitFrames.begin() + i);
			--i;
		}
	}
}

void __stdcall AdjustAngles_Hooked(int nSlot, float frametime)
{
	AdjustAngles_Original(nSlot, frametime);

	if (!pCmd)
		return;

	QAngle viewangles;
	g_pEngineClient->GetViewAngles(viewangles);

	bool bYawChanged = false;
	bool bOnGround = false;

	if (g_InputState.bSetAngles)
	{
		bool pitchChanged = DoAngleChange2(viewangles[PITCH], g_InputState.setAngles[PITCH], g_InputState.setAnglesSpeed[PITCH]);
		bool yawChanged = bYawChanged = DoAngleChange2(viewangles[YAW], g_InputState.setAngles[YAW], g_InputState.setAnglesSpeed[YAW]);

		if (!pitchChanged && !yawChanged)
			g_InputState.bSetAngles = false;
	}
	else
	{
		if (g_InputState.bSetPitch)
		{
			g_InputState.bSetPitch = DoAngleChange(viewangles[PITCH], tas_setpitch.GetFloat());
		}

		if (g_InputState.bSetYaw)
		{
			g_InputState.bSetYaw = bYawChanged = DoAngleChange(viewangles[YAW], tas_setyaw.GetFloat());
		}
	}

	if (tas_strafe.GetBool())
	{
		const int IN_JUMP = 1 << 1;

		Strafe::MovementVars vars = GetMovementVars();
		Strafe::PlayerData pl = GetPlayerData();

		if (vars.OnGround)
			++nTicksOnGround;

		if (!(tas_strafe_ignore_ground.GetBool() && vars.OnGround))
		{
			Strafe::StrafeButtons btns = Strafe::StrafeButtons();

			bool usingButtons = false;
			unsigned long buttons = atol(tas_strafe_buttons.GetString());

			if (buttons)
			{
				btns.GroundRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
				btns.GroundLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
				btns.AirRight = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;
				btns.AirLeft = static_cast<Strafe::Button>(buttons % 10); buttons /= 10;

				usingButtons = true;
			}

			Strafe::ProcessedFrame out;

			if (tas_strafe_autojump.GetBool() && Strafe::AutoJump(pl, vars))
				g_InputState.bForceJump = true;

			Friction(pl, vars.OnGround, vars);

			if (tas_strafe_vectorial.GetBool())
				Strafe::StrafeVectorial(pl, vars, *tas_strafe_yaw.GetString() != '\0' ? tas_strafe_yaw.GetFloat() : viewangles[YAW], viewangles[YAW], out, bYawChanged);
			else if (!bYawChanged)
				Strafe::Strafe(pl, vars, tas_strafe_yaw.GetFloat(), viewangles[YAW], out, btns, usingButtons);

			if (out.Processed)
			{
				pCmd->forwardmove = out.ForwardSpeed;
				pCmd->sidemove = out.SideSpeed;
				viewangles[YAW] = static_cast<float>(out.Yaw);
			}
		}
	}
	else if (GetGroundEntity(s_pLocalPlayer[0]))
	{
		++nTicksOnGround;
	}

	g_pEngineClient->SetViewAngles(viewangles);
}

bool __fastcall CheckJumpButton_Hooked(void *thisptr, int edx)
{
	bool bJump = CheckJumpButton_Original(thisptr);

	if (bJump)
	{
		float flCurrentSpeed = reinterpret_cast<Vector *>(GetOffset(s_pLocalPlayer[0], RecvPropOffsets::m_vecVelocity))->Length2D();

		if (nTicksOnGround <= 3)
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
		nTicksOnGround = 0;
	}

	return bJump;
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
	// s_pLocalPlayer
	void *pCheckForLocalPlayer = FIND_PATTERN(L"client.dll", Patterns::Client::C_BasePlayer__CheckForLocalPlayer);

	if (!pCheckForLocalPlayer)
	{
		FailedInit("C_BasePlayer::CheckForLocalPlayer");
		return false;
	}

	// CInput::GetButtonBits
	void *pGetButtonBits = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__GetButtonBits);

	if (!pGetButtonBits)
	{
		FailedInit("CInput::GetButtonBits");
		return false;
	}
	
	// CInput::CreateMove
	void *pCreateMove = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__CreateMove);

	if (!pCreateMove)
	{
		FailedInit("CInput::CreateMove");
		return false;
	}

	// CInput::AdjustAngles
	void *pAdjustAngles = FIND_PATTERN(L"client.dll", Patterns::Client::CInput__AdjustAngles);

	if (!pAdjustAngles)
	{
		FailedInit("CInput::AdjustAngles");
		return false;
	}

	// C_BaseEntity::GetGroundEntity
	if (!g_pCheckJumpButtonClient)
	{
		FailedInit("C_BaseEntity::GetGroundEntity");
		return false;
	}

	// s_pLocalPlayer
	s_pLocalPlayer = *reinterpret_cast<C_BasePlayer ***>(GetOffset(pCheckForLocalPlayer, Offsets::Variables::s_pLocalPlayer));

	// C_BaseEntity::GetGroundEntity
	DWORD dwRelativeAddress = *reinterpret_cast<DWORD *>(GetOffset(g_pCheckJumpButtonClient, Offsets::Functions::CBaseEntity__GetGroundEntity));
	DWORD dwCallAddress = (DWORD)(GetOffset(g_pCheckJumpButtonClient, Offsets::Functions::CBaseEntity__GetGroundEntity));
	GetGroundEntity = (GetGroundEntityFn)(dwRelativeAddress + dwCallAddress + sizeof(void *));

	// Get offsets of receive properties
	RecvPropOffsets::m_fFlags = RecvProps.GetPropOffset("CBasePlayer", "m_fFlags");
	RecvPropOffsets::m_vecVelocity = RecvProps.GetPropOffset("CBasePlayer", "m_vecVelocity[0]");
	RecvPropOffsets::m_vecOrigin = RecvProps.GetPropOffset("CBasePlayer", "m_vecOrigin");
	RecvPropOffsets::m_flMaxspeed = RecvProps.GetPropOffset("CBasePlayer", "m_flMaxspeed");
	RecvPropOffsets::m_flFriction = RecvProps.GetPropOffset("CBasePlayer", "m_flFriction");

	// Native cvars
	sv_airaccelerate = g_pCVar->FindVar("sv_airaccelerate");
	sv_accelerate = g_pCVar->FindVar("sv_accelerate");
	sv_friction = g_pCVar->FindVar("sv_friction");
	sv_maxspeed = g_pCVar->FindVar("sv_maxspeed");
	sv_stopspeed = g_pCVar->FindVar("sv_stopspeed");

	// Init hooks for trampoline
	INIT_HOOK(GetButtonBits_Hook, pGetButtonBits, GetButtonBits_Hooked);
	INIT_HOOK(CreateMove_Hook, pCreateMove, CreateMove_Hooked);
	INIT_HOOK(AdjustAngles_Hook, pAdjustAngles, AdjustAngles_Hooked);
	INIT_HOOK(CheckJumpButton_Hook, GetOffset(g_pCheckJumpButtonClient, -0x148), CheckJumpButton_Hooked);

	// Hook functions with trampoline method
	HOOK_FUNC(GetButtonBits_Hook, GetButtonBits_Original, GetButtonBitsFn);
	HOOK_FUNC(CreateMove_Hook, CreateMove_Original, CreateMoveFn);
	HOOK_FUNC(AdjustAngles_Hook, AdjustAngles_Original, AdjustAnglesFn);
	HOOK_FUNC(CheckJumpButton_Hook, CheckJumpButton_Original, CheckJumpButtonFn);

	// Hook vtable
	HOOK_VTABLE(IBaseClientDLL_Hook, g_pClient, Offsets::Functions::IBaseClientDLL__HudUpdate + 1);

	// Hook vtable function(s)
	HOOK_VTABLE_FUNC(IBaseClientDLL_Hook, HudUpdate_Hooked, Offsets::Functions::IBaseClientDLL__HudUpdate, HudUpdate_Original, HudUpdateFn);

	__INITIALIZED__ = true;
	return true;
}

void ReleaseClientModule()
{
	if (!__INITIALIZED__)
		return;

	// Unhook functions
	UNHOOK_FUNC(GetButtonBits_Hook);
	UNHOOK_FUNC(CreateMove_Hook);
	UNHOOK_FUNC(AdjustAngles_Hook);
	UNHOOK_FUNC(CheckJumpButton_Hook);

	// Remove hooks
	REMOVE_HOOK(GetButtonBits_Hook);
	REMOVE_HOOK(CreateMove_Hook);
	REMOVE_HOOK(AdjustAngles_Hook);
	REMOVE_HOOK(CheckJumpButton_Hook);

	// Unhook vtable functions
	UNHOOK_VTABLE_FUNC(IBaseClientDLL_Hook, Offsets::Functions::IBaseClientDLL__HudUpdate);

	// Remove vtable hook
	REMOVE_VTABLE_HOOK(IBaseClientDLL_Hook);
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(wait_frames, "Execute a given command after a certain number of game frames")
{
	if (args.ArgC() != 3)
	{
		Msg("Usage: wait_frames [count] [command]\n");
		return;
	}

	long long nFrames = atoll(args.Arg(1));

	if (nFrames <= 0)
	{
		// void IVEngineClient::ExecuteClientCmd( const char *szCmdString ); \ ecx is not used
		GetVTableFunction<void (__stdcall *)(const char *)>(g_pEngineClient, Offsets::Functions::IVEngineClient__ExecuteClientCmd)(args.Arg(2));
		return;
	}

	const char *szCommand = args.Arg(2);
	size_t length = strlen(szCommand);

	if (length > 0)
	{
		CWaitFrame *frame_cmd = new CWaitFrame(nFrames, szCommand, length);
		g_vecWaitFrames.push_back(frame_cmd);
	}
}

CON_COMMAND(wait_frames_clear_queue, "Clear the queue of wait_frames")
{
	for (size_t i = 0; i < g_vecWaitFrames.size(); ++i)
		delete g_vecWaitFrames[i];

	g_vecWaitFrames.clear();
}

CON_COMMAND(tas_create_trigger, "Create a trigger for listening client's position in the given box coordinates")
{
	if (args.ArgC() != 8)
	{
		Msg("Usage: tas_create_trigger [exec. file] [start: x] [start: y] [start: z] [end: x] [end: y] [end: z]\n");
		return;
	}

	const char *pszTriggerOutput = args[1];

	if (strlen(pszTriggerOutput) + 1 > TRIGGER_OUTPUT_MAXLEN)
	{
		Msg("tas_create_trigger: buffer overflow\n");
		return;
	}

	Vector vecStart = { static_cast<float>(atof(args[2])), static_cast<float>(atof(args[3])), static_cast<float>(atof(args[4])) };
	Vector vecEnd = { static_cast<float>(atof(args[5])), static_cast<float>(atof(args[6])), static_cast<float>(atof(args[7])) };

	CSimpleTrigger *trigger = new CSimpleTrigger(args[1], vecStart, vecEnd);
	g_vecTriggers.push_back(trigger);
}

CON_COMMAND(tas_remove_triggers, "Remove all triggers")
{
	for (size_t i = 0; i < g_vecTriggers.size(); ++i)
		delete g_vecTriggers[i];

	g_vecTriggers.clear();
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
		QAngle viewangles;
		g_pEngineClient->GetViewAngles(viewangles);

		float flPitch = atof(args.Arg(1));
		float flYaw = atof(args.Arg(2));

		float flNormalizedPitch = Strafe::NormalizeDeg(static_cast<double>(flPitch) - viewangles[PITCH]);
		float flNormalizedYaw = Strafe::NormalizeDeg(static_cast<double>(flYaw) - viewangles[YAW]);

		g_InputState.setAngles[PITCH] = flPitch;
		g_InputState.setAngles[YAW] = flYaw;

		g_InputState.setAnglesSpeed[PITCH] = std::abs(flNormalizedPitch) / nFrames;
		g_InputState.setAnglesSpeed[YAW] = std::abs(flNormalizedYaw) / nFrames;

		g_InputState.bSetAngles = true;
	}
}