// Server Module

#pragma once

#include "../game/defs.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CUserCmd;
class IMoveHelper;
class IGameMovement;
class IGameEvent;

class CBasePlayer;
class CTerrorPlayer;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

enum ServerCallbacks
{
	CB_ROUND_RESTART = 0,
	CB_MAP_LOADED,
	CB_INTRO_FINISHED,
	CB_SERVER_TRANSITION_FINISHED,
	CB_TRANSITION_FINISHED,
	CB_TIMER_STARTED,
	CB_TIMER_STOPPED
};

//-----------------------------------------------------------------------------
// Class CGlobalEntityList
//-----------------------------------------------------------------------------

class CGlobalEntityList
{
public:
	inline CBaseEntity *FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);
	inline CBaseEntity *FindEntityByClassnameFast(CBaseEntity *pStartEntity, const char *szName);
};

//-----------------------------------------------------------------------------
// Some exports
//-----------------------------------------------------------------------------

extern CGlobalEntityList *gEntList;

void server__tas_im_record(const char *pszFilename, int nPlayerIndex);
void server__tas_im_play(const char *pszFilename, int nPlayerIndex);
void server__tas_im_split(int nPlayerIndex);
void server__tas_im_stop(int nPlayerIndex);

//-----------------------------------------------------------------------------
// Server Module
//-----------------------------------------------------------------------------

class CServer
{
public:
	CServer();

	bool Init();
	bool Release();

	bool IsInitialized() const;

public:
	// Fire a few callback methods: execute a config file and call a VScript function
	void __declspec(noinline) FireServerCallback(ServerCallbacks type, bool bTimerStoppedOutsidePlugin = false);

public: // Hooks handlers
	// Handle call of CGameMovement::CheckJumpButton
	bool CheckJumpButton(IGameMovement *pGameMovement);

public:
	inline bool IsTransitioning() const;
	inline bool IsSegmentFinished() const;

	inline void SetTransitionState(bool bState);
	inline void SetSegmentFinishedState(bool bState);

	// Auto jump for a specified client
	inline bool IsAutoJumpEnabled(int nPlayerIndex);

	inline void EnableAutoJump(int nPlayerIndex);
	inline void DisableAutoJump(int nPlayerIndex);

	inline void ToggleAutoJump(int nPlayerIndex);

private:
	bool m_bInitialized;
	bool m_bAutoJump[MAXCLIENTS + 1];

	bool m_bInTransition;
	bool m_bSegmentFinished;
};

inline bool CServer::IsTransitioning() const { return m_bInTransition; }
inline bool CServer::IsSegmentFinished() const { return m_bSegmentFinished; }

inline void CServer::SetTransitionState(bool bState) { m_bInTransition = bState; }
inline void CServer::SetSegmentFinishedState(bool bState) { m_bSegmentFinished = bState; }

inline bool CServer::IsAutoJumpEnabled(int nPlayerIndex)
{
	return m_bAutoJump[nPlayerIndex];
}

inline void CServer::EnableAutoJump(int nPlayerIndex)
{
	m_bAutoJump[nPlayerIndex] = true;
}

inline void CServer::DisableAutoJump(int nPlayerIndex)
{
	m_bAutoJump[nPlayerIndex] = false;
}

inline void CServer::ToggleAutoJump(int nPlayerIndex)
{
	m_bAutoJump[nPlayerIndex] = !m_bAutoJump[nPlayerIndex];
}

extern CServer g_Server;