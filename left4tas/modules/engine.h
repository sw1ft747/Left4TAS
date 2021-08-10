// C++
// Engine Module

#pragma once

#include "../sdk.h"

#include "../structs/cmd.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef void (__thiscall *SetPausedFn)(void *, bool);

typedef bool (__cdecl *Host_NewGameFn)(char *, bool, bool, bool, const char *, const char *);
typedef void (__cdecl *Host_ChangelevelFn)(bool, const char *, const char *);

typedef void (__thiscall *PaintFn)(void *, PaintMode_t);

//-----------------------------------------------------------------------------

extern bool g_bLevelChange;
extern void *g_pGetBaseLocalClient;

extern Cbuf_AddTextFn Cbuf_AddText;
extern Cbuf_ExecuteFn Cbuf_Execute;

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsEngineModuleInit();

bool InitEngineModule();

void ReleaseEngineModule();