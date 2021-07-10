// C++
// Offsets

#include "offsets.h"

namespace Offsets
{
	namespace Functions
	{
		OFFSET(IServer__IsPaused, 25);
		OFFSET(IServer__SetPaused, 29);

		OFFSET(IVEngineClient__ExecuteClientCmd, 103);

		OFFSET(CBaseClient__SetName, 16);
		OFFSET(CBaseClient__SetUserCVar, 17);

		OFFSET(IBaseClientDLL__GetAllClasses, 7);
		OFFSET(IBaseClientDLL__HudUpdate, 10);

		OFFSET(IPanel__GetName, 36);
		OFFSET(IPanel__PaintTraverse, 41);

		OFFSET(CBaseEntity__Teleport, 117);
		OFFSET2(CBaseEntity__GetGroundEntity, -22);
	}

	namespace Variables
	{
		OFFSET(spszVersionString, -0x5);

		OFFSET(CBaseServer__SpawnServer, 0xA);

		OFFSET(s_pLocalPlayer, 0x51);

		OFFSET(g_pScriptVM, 0x17);
		OFFSET(g_pScriptManager, 0x35);

		OFFSET(s_UniformStream, 0x15);

		OFFSET(m_pCommands, 0xDC);
	}
}