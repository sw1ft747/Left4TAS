// C++
// Patterns

#include "patterns.h"

namespace Patterns
{
	namespace Engine
	{
		PATTERN(Host_NewGame, "55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 8B 45 18 8B 4D 1C 53 56 8B 75 08 57");
		PATTERN(Host_Changelevel, "55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 53 8B 5D 10 56 8B 75 0C 57 33 FF");
	}

	namespace Client
	{
		PATTERN(C_BasePlayer__CheckForLocalPlayer, "55 8B EC 56 57 8B 7D 08 8B F1 83 FF FF 74 ? 8B 0D");

		PATTERN(CInput__GetButtonBits, "55 8B EC 83 EC 08 56 8B F1 8B 0D ? ? ? ? 8B 01 8B 90 ? ? 00 00 57 89 75 F8");
		PATTERN(CInput__CreateMove, "55 8B EC 83 EC 18 53 56 57 8B F9 8B 0D");
		PATTERN(CInput__AdjustAngles, "55 8B EC 83 EC 0C D9 45 0C 56 57 8B 7D 08 51");

		PATTERN(CGameMovement__CheckJumpButton, "84 58 28 75 F3 6A 00 8B CE E8");
	}

	namespace Server
	{
		PATTERN(VScriptServerInit, "55 8B EC 51 53 56 33 F6 57 39 35 ? ? ? ? 0F 84 ? ? ? ? 8B 1D");
		PATTERN(VScriptServerTerm, "80 3D ? ? ? ? 00 74 0C E8 ? ? ? ? C6 05 ? ? ? ? 01 83 3D");

		PATTERN(CBaseEntity__AcceptInput, "55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 8B 45 0C 53 8B 5D 10 56 57");

		PATTERN(CBasePlayer__PlayerRunCommand, "55 8B EC 56 8B 75 08 57 8B F9 33 C0 88 87 ? ? ? ? 39 87");

		PATTERN(CTerrorPlayer__GoAwayFromKeyboard, "55 8B EC 83 EC 08 53 56 57 8B F1 8B 06 8B 90 ? ? ? ? 8B 9E");
		PATTERN(CTerrorPlayer__TakeOverBot, "55 8B EC 81 EC ? ? ? ? A1 ? ? ? ? 33 C5 89 45 FC 53 56 8D 85");

		PATTERN(UTIL_PlayerByIndex, "55 8B EC 8B 45 08 57 33 FF 85 C0 7E 50 8B 0D");

		PATTERN(CDirectorChallengeMode__TeamStartTouchIsValid, "55 8B EC 53 56 8B F1 8B 86 ? ? ? ? 83 F8 FF 74");
		PATTERN(CDirectorSessionManager__UnfreezeTeam, "55 8B EC 83 EC 4C 53 56 57 33 FF 8D 45 B4 8B F1 50");

		PATTERN(CDirector__OnStartIntro, "8B 0D ? ? ? ? 56 8B 35 ? ? ? ? 6A 01 E8");
		PATTERN(CDirector__OnFinishIntro, "55 8B EC 51 8B 0D ? ? ? ? 56 8B 35 ? ? ? ? 6A 00 E8");
		PATTERN(CDirector__OnBeginTransition, "55 8B EC 83 EC 28 80 7D 08 00 53 56 8B F1 0F 94 C0");
		PATTERN(CDirector__OnFinaleEscapeForceSurvivorPositions, "55 8B EC 51 53 56 8B 75 08 57 8B F9 8B 8F");

		PATTERN(CFinaleTrigger__OnFinaleEscapeForceSurvivorPositions, "55 8B EC 83 EC 2C A1 ? ? ? ? 53 8B D9 8B 88");

		PATTERN(CTerrorGameRules__RestartRound, "55 8B EC 0F 57 C0 83 EC 08 53 33 DB 56 57 8B F9");

		PATTERN(CGlobalEntityList__FindEntityByClassname, "55 8B EC 53 56 8B F1 8B 4D 08 57 85 C9 74 16 8B 01 8B 50 08 FF D2 8B 00 25 FF 0F 00 00 40 03 C0 8B 3C C6");
		PATTERN(CGlobalEntityList__FindEntityByClassnameFast, "55 8B EC 8B 45 08 83 EC 0C 85 C0 74 0C 8B 80 ? ? ? ? 8B E5 5D C2 08 00 8B 45 0C 8D 4D FC 51");

		PATTERN(CGameMovement__CheckJumpButton, "84 58 28 75 F3 6A 00 8B CE E8");

		PATTERN(ForEachTerrorPlayer_FindCharacter, "55 8B EC A1 ? ? ? ? 53 56 57 BF 01 00 00 00 39 78 14 7C 6C 8B 5D 08 57 E8");
	}

	namespace VSTDLib
	{
		PATTERN(InstallUniformRandomStream, "55 8B EC 8B 45 08 A3 ? ? ? ? 85 C0 75 0A C7 05 ? ? ? ? ? ? ? ? 5D C3");
	}
}