// C++
// Engine Module

#pragma once

#include "../sdk.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef void (__thiscall *SetPausedFn)(void *, bool);

typedef bool (__cdecl *Host_NewGameFn)(char *, bool, bool, bool, const char *, const char *);
typedef void (__cdecl *Host_ChangelevelFn)(bool, const char *, const char *);

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsEngineModuleInit();

bool InitEngineModule();

void ReleaseEngineModule();