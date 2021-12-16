// VScript Module

#pragma once

#include "../sdk.h"
#include "../vscript/ivscript.h"

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern IScriptVM *g_pScriptVM;
extern IScriptManager *g_pScriptManager;

extern HSCRIPT g_hScriptL4TAS;

//-----------------------------------------------------------------------------
// VScript Module
//-----------------------------------------------------------------------------

class CVScript
{
public:
	CVScript();

	bool Init();
	bool Release();

	bool IsInitialized() const;

public:
	inline IScriptVM **GetScriptVMPointer() const;

	void InitVScriptBridge();
	void TermVScriptBridge();

private:
	bool m_bInitialized;
	IScriptVM **m_ppScriptVM;
};

inline IScriptVM **CVScript::GetScriptVMPointer() const { return m_ppScriptVM; }

extern CVScript g_VScript;