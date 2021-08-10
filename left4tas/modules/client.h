// C++
// Client Module

#pragma once

#include "../sdk.h"
#include "../offsets.h"
#include "../utils/utils.h"

#include <vector>

#define MAX_SPLITSCREEN_PLAYERS 2

//-----------------------------------------------------------------------------

class C_BaseEntity;
class C_BasePlayer;
class CUserCmd;
class IClientMode;

//-----------------------------------------------------------------------------
// Typedefs for hooks
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

typedef bool (__thiscall *CheckJumpButtonClientFn)(void *);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CWaitFrame
{
public:
	CWaitFrame(long long nFrames, const char *szCommand);
	~CWaitFrame();

	long long m_nFrames;
	const char *m_szCommand;
};

struct input_state
{
	input_state() : bForceJump(false), bSetAngles(false), bSetPitch(false), bSetYaw(false)
	{
		setAngles.Init();
		setAnglesSpeed.Init();
	}

	Vector2D setAngles;
	Vector2D setAnglesSpeed;

	bool bForceJump;
	bool bSetAngles;
	bool bSetPitch;
	bool bSetYaw;
};

struct bhop_info
{
	bhop_info() : nJumps(0), flSpeedLoss(0.0f), flPercentage(100.0f), flLastSpeed(0.0f)
	{
	}

	unsigned long nJumps;
	float flSpeedLoss;
	float flPercentage;
	float flLastSpeed;
};

//-----------------------------------------------------------------------------

extern std::vector<CWaitFrame *> g_vecWaitFrames;

extern bool g_bAutoJumpClient[MAX_SPLITSCREEN_PLAYERS];
extern input_state g_InputState[MAX_SPLITSCREEN_PLAYERS];
extern IClientMode **g_pClientMode;
extern C_BasePlayer **s_pLocalPlayer;
extern bhop_info g_bhopInfo;

extern int g_nForceUser;
extern bool g_bInSplitScreen;

void GetViewAngles(int nSlot, QAngle &va);
void SetViewAngles(int nSlot, QAngle &va);

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsClientModuleInit();

bool InitClientModule();

void ReleaseClientModule();