// Offsets

#include "offsets.h"

namespace Offsets
{
	namespace Functions
	{
		OFFSET(IServer__IsPaused, 25);
		OFFSET(IServer__SetPaused, 29);

		OFFSET(IVEngineClient__ExecuteClientCmd, 103);
		OFFSET(IVEngineClient__GetActiveSplitScreenPlayerSlot, 126);

		OFFSET(IBaseClientDLL__GetAllClasses, 7);
		OFFSET(IBaseClientDLL__HudUpdate, 10);

		OFFSET(IEngineVGuiInternal__Paint, 14);

		OFFSET(IClientMode__CreateMove, 27);

		OFFSET(ISplitScreen__AddSplitScreenUser, 2);
		OFFSET(ISplitScreen__RemoveSplitScreenUser, 4);

		OFFSET(CBaseClient__SetName, 16);
		OFFSET(CBaseClient__SetUserCVar, 17);

		//OFFSET(IPanel__GetName, 36);
		//OFFSET(IPanel__PaintTraverse, 41);

		OFFSET2(CMatSystemSurface__StartDrawing, -27);
		OFFSET2(CMatSystemSurface__FinishDrawing, -17);

		OFFSET2(CCSModeManager__Init, -0x39);

		OFFSET(CBaseEntity__AcceptInput, 43);
		OFFSET(CBaseEntity__Teleport, 117);

		OFFSET2(C_BaseEntity__GetGroundEntity, 0x131);

		// Might be CWeaponCSBase::GetWeaponID
		OFFSET(CTerrorWeapon__GetWeaponID, 396);
		OFFSET(C_TerrorWeapon__GetWeaponID, 383);
	}

	namespace Variables
	{
		OFFSET(spszVersionString, -0x5);

		OFFSET(sv, 0x9);

		OFFSET(s_pLocalPlayer, 0x4F);

		OFFSET(g_pScriptVM, 0x15);
		OFFSET(g_pScriptManager, 0x33);

		OFFSET(g_pClientMode, 0x10);

		OFFSET(s_UniformStream, 0x15);

		OFFSET(m_pCommands, 0xA8);

		OFFSET(m_Client__viewangles, 19108);
	}
}