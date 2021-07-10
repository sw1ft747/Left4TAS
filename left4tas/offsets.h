// C++
// Offsets

#pragma once

namespace Offsets
{
	#define INIT_OFFSET(name) extern unsigned long name;
	#define INIT_OFFSET2(name) extern signed long name;

	#define OFFSET(name, offset) unsigned long name = offset;
	#define OFFSET2(name, offset) signed long name = offset;

	namespace Functions
	{
		INIT_OFFSET(IServer__IsPaused);
		INIT_OFFSET(IServer__SetPaused);

		INIT_OFFSET(IVEngineClient__ExecuteClientCmd);

		INIT_OFFSET(CBaseClient__SetName);
		INIT_OFFSET(CBaseClient__SetUserCVar);

		INIT_OFFSET(IBaseClientDLL__GetAllClasses);
		INIT_OFFSET(IBaseClientDLL__HudUpdate);

		INIT_OFFSET(IPanel__GetName);
		INIT_OFFSET(IPanel__PaintTraverse);

		INIT_OFFSET(CBaseEntity__Teleport);
		INIT_OFFSET2(CBaseEntity__GetGroundEntity);
	}

	namespace Variables
	{
		INIT_OFFSET(spszVersionString);

		INIT_OFFSET(CBaseServer__SpawnServer);

		INIT_OFFSET(s_pLocalPlayer);

		INIT_OFFSET(g_pScriptVM);
		INIT_OFFSET(g_pScriptManager);

		INIT_OFFSET(s_UniformStream);

		INIT_OFFSET(m_pCommands);
		INIT_OFFSET(m_pCommands_v2213);
	}
}