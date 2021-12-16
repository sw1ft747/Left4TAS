// Client Module

#pragma once

#include "../sdk.h"
#include "../offsets.h"
#include "../utils/utils.h"

#include "../game/defs.h"
#include "../game/cmd.h"

#include "../tools/strafe.h"
#include "../tools/simple_trigger.h"

#include <vector>

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class C_BaseEntity;
class C_BasePlayer;
class CClientState;
class CUserCmd;
class IClientMode;
class SplitPlayer_t;
class IGameMovement;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CQueuedCommand
{
public:
	CQueuedCommand(long long nFrames, const char *pszCommand);
	~CQueuedCommand();

	long long m_nFrames;
	const char *m_pszCommand;
};

class CInputAction
{
public:
	CInputAction() : m_bSetAngles(false), m_bSetPitch(false), m_bSetYaw(false)
	{
		m_vecSetAngles.Init();
		m_vecSetAnglesSpeed.Init();
	}

	Vector2D m_vecSetAngles;
	Vector2D m_vecSetAnglesSpeed;

	bool m_bSetAngles;
	bool m_bSetPitch;
	bool m_bSetYaw;
};

class CBunnyhopInfo
{
public:
	CBunnyhopInfo() : m_nJumps(0), m_flSpeedLoss(0.0f), m_flPercentage(100.0f), m_flLastSpeed(0.0f)
	{
	}

	unsigned long m_nJumps;
	float m_flSpeedLoss;
	float m_flPercentage;
	float m_flLastSpeed;
};

//-----------------------------------------------------------------------------
// Client Module
//-----------------------------------------------------------------------------

class CClient
{
public:
	CClient();

	bool Init();
	bool Release();

	bool IsInitialized() const;

public:
	// Process all triggers
	void ProcessTriggers();

	// Create a trigger to listen local player's position
	void CreateTrigger(const char *pszExecFile, Vector &vecStart, Vector &vecEnd, int iData);

	// Remove all triggers
	void RemoveTriggers();
	
	// Remove all triggers for a specified splitscreen client
	void RemoveTriggersForClient(int nSlot);

public:
	// Process (execute) all queued commands
	void ProcessQueuedCommands();

	// Add a queued command for an active splitscreen client
	void AddQueuedCommand(long long nFrames, const char *pszCommand);

	// Add a queued command for a specified client
	void AddQueuedCommand(int nSlot, long long nFrames, const char *pszCommand);

	// Clear all queued commands for all splitscreen clients
	void ClearQueuedCommands();

	// Clear all queued commands for a specified client
	void ClearQueuedCommands(int nSlot);

public: // Hooks handlers
	// Process user's inputs: angle change, strafing, etc.
	void ProcessInputs(int nSlot);

	// Handle call of CGameMovement::CheckJumpButton
	bool CheckJumpButton(IGameMovement *pGameMovement);

public:
	// Update strafe data for a specified client
	void UpdateStrafeData(int nSlot);

	// Get/set engine's view angles
	void GetViewAngles(int nSlot, QAngle &va);
	void SetViewAngles(int nSlot, QAngle &va);

	// Execute client command in this frame
	void ExecuteClientCmd(int nSlot, const char *pszCommand);
	void ExecuteClientCmd(ECommandTarget_t target, const char *pszCommand);

	// Change angle depending by speed given in cvar 'tas_setanglespeed'
	bool ChangeAngle(float &flAngle, float flTargetAngle);

	// Change angle by given speed
	bool ChangeAngleBySpeed(float &flAngle, float flTargetAngle, float flChangeSpeed);

	// Auto jump for a specified client
	inline bool IsAutoJumpEnabled(int nSlot);

	inline void EnableAutoJump(int nSlot);
	inline void DisableAutoJump(int nSlot);

	inline void ToggleAutoJump(int nSlot);

public:
	inline C_BasePlayer *GetLocalPlayer(int nSlot) const;

	inline CClientState *GetLocalClient(int nSlot) const;
	inline IClientMode *GetClientMode(int nSlot) const;

	inline Strafe::StrafeData &GetStrafeData(int nSlot) const;

	inline CInputAction &GetInputAction(int nSlot) const;
	inline CBunnyhopInfo &GetBunnyhopInfo() const;

public:
	int m_nForceUser;
	bool m_bInSplitScreen;

private:
	bool m_bInitialized;
	bool m_bAutoJump[MAX_SPLITSCREEN_PLAYERS];

	Strafe::StrafeData m_StrafeData[MAX_SPLITSCREEN_PLAYERS];
	CInputAction m_InputAction[MAX_SPLITSCREEN_PLAYERS];
	CBunnyhopInfo m_BunnyhopInfo;

	std::vector<CQueuedCommand *> m_vecQueuedCommands[MAX_SPLITSCREEN_PLAYERS];
	std::vector<CSimpleTrigger *> m_vecTriggers;

	C_BasePlayer **m_pLocalPlayer;
	IClientMode **m_pClientMode;
	SplitPlayer_t **m_pSplitScreenPlayers;
};

inline bool CClient::IsAutoJumpEnabled(int nSlot)
{
	return m_bAutoJump[nSlot];
}

inline void CClient::EnableAutoJump(int nSlot)
{
	m_bAutoJump[nSlot] = true;
}

inline void CClient::DisableAutoJump(int nSlot)
{
	m_bAutoJump[nSlot] = false;
}

inline void CClient::ToggleAutoJump(int nSlot)
{
	m_bAutoJump[nSlot] = !m_bAutoJump[nSlot];
}

inline C_BasePlayer *CClient::GetLocalPlayer(int nSlot) const { return m_pLocalPlayer[nSlot]; }
inline CClientState *CClient::GetLocalClient(int nSlot) const { return (CClientState *)GetOffset(m_pSplitScreenPlayers[nSlot], 8); }
inline IClientMode *CClient::GetClientMode(int nSlot) const { return m_pClientMode[nSlot]; }

inline Strafe::StrafeData &CClient::GetStrafeData(int nSlot) const { return const_cast<Strafe::StrafeData &>(m_StrafeData[nSlot]); }
inline CInputAction &CClient::GetInputAction(int nSlot) const { return const_cast<CInputAction &>(m_InputAction[nSlot]); }
inline CBunnyhopInfo &CClient::GetBunnyhopInfo() const { return const_cast<CBunnyhopInfo &>(m_BunnyhopInfo); }

extern CClient g_Client;