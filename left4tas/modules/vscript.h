// C++
// VScript Module

#pragma once

#include "../sdk.h"
#include "../vscript/ivscript.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef bool (*VScriptServerInitFn)();
typedef void (*VScriptServerTermFn)();

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

extern IScriptVM *g_pScriptVM;
extern IScriptManager *g_pScriptManager;

extern HSCRIPT g_hScriptL4TAS;

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsVScriptModuleInit();

bool InitVScriptModule();

void ReleaseVScriptModule();