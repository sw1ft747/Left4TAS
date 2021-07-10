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
	}

	namespace Client
	{
		INIT_PATTERN(C_BasePlayer__CheckForLocalPlayer);

		INIT_PATTERN(CInput__GetButtonBits);
		INIT_PATTERN(CInput__CreateMove);
		INIT_PATTERN(CInput__AdjustAngles);

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

		INIT_PATTERN(CDirectorChallengeMode__TeamStartTouchIsValid);
		INIT_PATTERN(CDirectorSessionManager__UnfreezeTeam);

		INIT_PATTERN(CDirector__OnStartIntro);
		INIT_PATTERN(CDirector__OnFinishIntro);
		INIT_PATTERN(CDirector__OnBeginTransition);
		INIT_PATTERN(CDirector__OnFinaleEscapeForceSurvivorPositions);
		INIT_PATTERN(CDirector__OnFinaleEscapeForceSurvivorPositions_v2213);

		INIT_PATTERN(CFinaleTrigger__OnFinaleEscapeForceSurvivorPositions);
		INIT_PATTERN(CFinaleTrigger__OnFinaleEscapeForceSurvivorPositions_v2213);

		INIT_PATTERN(CTerrorGameRules__RestartRound);

		INIT_PATTERN(CGlobalEntityList__FindEntityByClassname);
		INIT_PATTERN(CGlobalEntityList__FindEntityByClassnameFast);

		INIT_PATTERN(CGameMovement__CheckJumpButton);

		INIT_PATTERN(ForEachTerrorPlayer_FindCharacter);
	}

	namespace VSTDLib
	{
		INIT_PATTERN(InstallUniformRandomStream);
	}
}