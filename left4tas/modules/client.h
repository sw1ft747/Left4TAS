// C++
// Client Module

#pragma once

#include "../sdk.h"
#include "../offsets.h"
#include "../utils/utils.h"

#include <vector>

//-----------------------------------------------------------------------------
// Typedefs for hooks
//-----------------------------------------------------------------------------

typedef C_BaseEntity *(__thiscall *GetGroundEntityFn)(void *);
typedef void (__stdcall *HudUpdateFn)(bool); // ecx is not used
typedef int (__thiscall *GetButtonBitsFn)(void *, bool);
typedef void (__thiscall *CreateMoveFn)(void *, int, float, bool);
typedef void (__stdcall *AdjustAnglesFn)(int, float); // ecx is not used
typedef bool (__thiscall *CheckJumpButtonFn)(void *);

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CWaitFrame
{
public:
	CWaitFrame(long long nFrames, const char *szCommand, size_t length);
	~CWaitFrame();

	long long m_nFrames;
	char *m_szCommand;
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
	bhop_info() : nJumps(0), flSpeedLoss(0.0f), flPercentage(0.0f), flLastSpeed(0.0f)
	{
	}

	unsigned long nJumps;
	float flSpeedLoss;
	float flPercentage;
	float flLastSpeed;
};

extern std::vector<CWaitFrame *> g_vecWaitFrames;
extern input_state g_InputState;

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsClientModuleInit();

bool InitClientModule();

void ReleaseClientModule();