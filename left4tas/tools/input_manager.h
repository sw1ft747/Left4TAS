// Input Manager

#pragma once

#include <stdio.h>

#include "../game/defs.h"

//-----------------------------------------------------------------------------
// Defs
//-----------------------------------------------------------------------------

#define INPUT_MANAGER_VERSION 1

#define IM_FILE_HEADER 0x2F4A
#define IM_FILE_INFO_HEADER 0xFC
#define IM_FILE_FRAMES_HEADER 0xFF

#define IM_BASE_INFO_SIZE sizeof(InputBaseInfo)
#define IM_FRAME_SIZE sizeof(InputFrame)

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CUserCmd;

//-----------------------------------------------------------------------------
// Extra structs
//-----------------------------------------------------------------------------

struct InputBaseInfo
{
	int				version;
	int				unused;

	// Start origin, viewangles, velocity
	float			origin[3];
	float			viewangles[3];
	float			velocity[3];
};

struct InputFrame
{
	float			viewangles[3];
	float			forwardmove;
	float			sidemove;
	int				buttons;
	unsigned char	impulse;
	unsigned char	weaponselect; // ID
	unsigned char	weaponsubtype; // always 0
	short			mousedx;
	short			mousedy;
};

struct InputData
{
	FILE			*input = NULL;
	const char		*filename = NULL;
	bool			active = false;
	bool			recording = true;
	int				frames = 0;

	InputBaseInfo	baseInfo;
};

//-----------------------------------------------------------------------------
// Main class
//-----------------------------------------------------------------------------

class CInputManager
{
public:
	void Init();

	// Get a splitscreen player's input data (client-side)
	inline InputData *GetClientInputData(int nSlot);

	// Get a player's input data (server-side)
	inline InputData *GetServerInputData(int nPlayerIndex);

public:
	void Record(const char *pszFilename, InputData *inputData, float orientation[3][3] /* origin, viewangles, velocity */);
	void Playback(const char *pszFilename, InputData *inputData);
	void Split(InputData *inputData);

	void Stop(InputData *inputData);

	void ReadInput(InputData *inputData, CUserCmd *pCmd, void *pPlayer, bool bLocalClient);
	void SaveInput(InputData *inputData, CUserCmd *pCmd, bool bLocalClient);

private:
	const char *GetFilePath(const char *pszFilename);

private:
	InputData m_InputDataClient[MAX_SPLITSCREEN_PLAYERS];
	InputData m_InputDataServer[MAXCLIENTS + 1];
};

inline InputData *CInputManager::GetClientInputData(int nSlot)
{
	return &m_InputDataClient[nSlot];
}

inline InputData *CInputManager::GetServerInputData(int nPlayerIndex)
{
	return &m_InputDataServer[nPlayerIndex];
}

extern CInputManager g_InputManager;