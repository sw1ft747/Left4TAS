// C++
// Patterns

#pragma once

#include "patterns_base.h"

namespace Patterns
{
	namespace Engine
	{
		INIT_PATTERN(Host_NewGame);
		INIT_PATTERN(Host_Changelevel);

		INIT_PATTERN(Cbuf_AddText);
		INIT_PATTERN(Cbuf_Execute);

		INIT_PATTERN(GetBaseLocalClient);
	}

	namespace Client
	{
		INIT_PATTERN(CCSModeManager__Init);

		INIT_PATTERN(C_BasePlayer__CheckForLocalPlayer);

		INIT_PATTERN(CInput__GetButtonBits);
		INIT_PATTERN(CInput__CreateMove);
		INIT_PATTERN(CInput__AdjustAngles);
		INIT_PATTERN(CInput__ControllerMove);

		INIT_PATTERN(CGameMovement__CheckJumpButton);
	}

	namespace Server
	{
		INIT_PATTERN(VScriptServerInit);
		INIT_PATTERN(VScriptServerTerm);

		INIT_PATTERN(CBaseEntity__AcceptInput);

		INIT_PATTERN(CBasePlayer__PlayerRunCommand);

		INIT_PATTERN(CTerrorPlayer__GoAwayFromKeyboard);
		INIT_PATTERN(CTerrorPlayer__TakeOverBot);

		INIT_PATTERN(UTIL_PlayerByIndex);

		INIT_PATTERN(RestoreTransitionedEntities);

		INIT_PATTERN(CDirectorChallengeMode__TeamStartTouchIsValid);

		INIT_PATTERN(CDirectorSessionManager__UnfreezeTeam);
		INIT_PATTERN(CDirectorSessionManager__FireGameEvent);

		INIT_PATTERN(CDirector__OnStartIntro);
		INIT_PATTERN(CDirector__OnFinishIntro);
		INIT_PATTERN(CDirector__OnBeginTransition);
		INIT_PATTERN(CDirector__OnFinaleEscapeForceSurvivorPositions);

		INIT_PATTERN(CFinaleTrigger__OnFinaleEscapeForceSurvivorPositions);

		INIT_PATTERN(CTerrorGameRules__RestartRound);

		INIT_PATTERN(CGlobalEntityList__FindEntityByClassname);
		INIT_PATTERN(CGlobalEntityList__FindEntityByClassnameFast);

		INIT_PATTERN(CGameMovement__CheckJumpButton);

		INIT_PATTERN(ForEachTerrorPlayer_FindCharacter);
	}

	namespace VGUIMatSurface
	{
		INIT_PATTERN(CMatSystemSurface__StartDrawing);
		INIT_PATTERN(CMatSystemSurface__FinishDrawing);
	}

	namespace VSTDLib
	{
		INIT_PATTERN(InstallUniformRandomStream);
	}
}