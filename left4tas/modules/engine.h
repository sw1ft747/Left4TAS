// Engine Module

#pragma once

#include "../sdk.h"

#include "../game/cmd.h"

//-----------------------------------------------------------------------------
// Engine Module
//-----------------------------------------------------------------------------

class CEngine
{
public:
	CEngine();

	bool Init();
	bool Release();

	bool IsInitialized() const;

public:
	inline bool IsLevelChanging() const;
	inline void SetLevelChangeState(bool bState);

	inline void *GetBaseLocalClientFunc() const;

public:
	Cbuf_AddTextFn Cbuf_AddText; // void Cbuf_AddTextFn(ECommandTarget_t eTarget, const char *text, cmd_source_t source)
	Cbuf_ExecuteFn Cbuf_Execute; // void Cbuf_ExecuteFn()

private:
	bool m_bInitialized;

	bool m_bLevelChange;
	void *m_pfnGetBaseLocalClient;
};

inline bool CEngine::IsLevelChanging() const { return m_bLevelChange; }

inline void CEngine::SetLevelChangeState(bool bState) { m_bLevelChange = bState; }

inline void *CEngine::GetBaseLocalClientFunc() const { return m_pfnGetBaseLocalClient; }

extern CEngine g_Engine;