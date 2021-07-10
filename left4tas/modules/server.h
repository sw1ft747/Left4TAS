// C++
// Server Module

#pragma once

#include "../game/server/variant_t.h"
#include "../sdk.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class CUserCmd;
class IMoveHelper;

class CBasePlayer;
class CTerrorPlayer;

struct inputdata_t
{
	CBaseEntity *pActivator;		// The entity that initially caused this chain of output events.
	CBaseEntity *pCaller;			// The entity that fired this particular output.
	variant_t value;				// The data parameter for this output.
	int nOutputID;					// The unique ID of the output that was fired.
};

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef CBaseEntity *(__thiscall *FindEntityByClassnameFn)(void *, CBaseEntity *, const char *);
typedef CBaseEntity *(__thiscall *FindEntityByClassnameFastFn)(void *, CBaseEntity *, string_t);

typedef void (__thiscall *PlayerRunCommandFn)(CBasePlayer *, CUserCmd *, IMoveHelper *);

typedef bool (__thiscall *TeamStartTouchIsValidFn)(void *, void *);

typedef uintptr_t (__thiscall *RestartRoundFn)(void *);
typedef uintptr_t (__thiscall *UnfreezeTeamFn)(void *);

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

extern CGlobalEntityList *gEntList;

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsServerModuleInit();

bool InitServerModule();

void ReleaseServerModule();