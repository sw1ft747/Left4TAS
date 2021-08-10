// C++
// Server Module

#pragma once

#include "../structs/inputdata_t.h"
#include "../sdk.h"

#define MAXCLIENTS 32

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CUserCmd;
class IMoveHelper;
class IGameEvent;

class CBasePlayer;
class CTerrorPlayer;

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
//-----------------------------------------------------------------------------

class CGlobalEntityList
{
public:
	inline CBaseEntity *FindEntityByClassname(CBaseEntity *pStartEntity, const char *szName);
	inline CBaseEntity *FindEntityByClassnameFast(CBaseEntity *pStartEntity, const char *szName);
};

//-----------------------------------------------------------------------------

extern bool g_bSegmentFinished;
extern bool g_bInTransition;

extern bool g_bAutoJumpServer[MAXCLIENTS + 1];
extern CGlobalEntityList *gEntList;

void server__tas_im_record(const char *pszFilename, int nPlayerIndex);
void server__tas_im_play(const char *pszFilename, int nPlayerIndex);
void server__tas_im_split(int nPlayerIndex);
void server__tas_im_stop(int nPlayerIndex);

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsServerModuleInit();

bool InitServerModule();

void ReleaseServerModule();