// User Messages

#include "usermessages.h"

bf_write *g_pMsgBuffer = NULL;
static CRecipientFilter s_pFilterBuffer;

const char *g_szUserMessages[] =
{
	"Geiger",
	"Train",
	"HudText",
	"SayText",
	"SayText2",
	"TextMsg",
	"HudMsg",
	"ResetHUD",
	"GameTitle",
	"ItemPickup",
	"ShowMenu",
	"Shake",
	"Fade",
	"VGUIMenu",
	"Rumble",
	"CloseCaption",
	"CloseCaptionDirect",
	"SendAudio",
	"RawAudio",
	"VoiceMask",
	"RequestState",
	"BarTime",
	"Damage",
	"RadioText",
	"HintText",
	"Key",
	"ReloadEffect",
	"PlayerAnimEvent",
	"AmmoDenied",
	"UpdateRadar",
	"KillCam",
	"MarkAchievement",
	"Splatter",
	"MeleeSlashSplatter",
	"MeleeClub",
	"MudSplatter",
	"SplatterClear",
	"MessageText",
	"TransitionRestore",
	"Spawn",
	"CreditsLine",
	"CreditsMsg",
	"JoinLateMsg",
	"StatsCrawlMsg",
	"StatsSkipState",
	"ShowStats",
	"BlurFade",
	"MusicCmd",
	"WitchBloodSplatter",
	"AchievementEvent",
	"PZDmgMsg",
	"AllPlayersConnectedGameStarting",
	"VoteRegistered",
	"DisconnectToLobby",
	"CallVoteFailed",
	"SteamWeaponStatData",
	"CurrentTimescale",
	"DesiredTimescale",
	"PZEndGamePanelMsg",
	"PZEndGamePanelVoteRegisteredMsg",
	"PZEndGameVoteStatsMsg",
	"VoteStart",
	"VotePass",
	"VoteFail"
};

void UserMessageBegin(UserMsgDest dest, const int nMsgType, const char *pszMessage, edict_t *pRecipient = NULL)
{
	switch (dest)
	{
	case MSG_UNICAST:
		s_pFilterBuffer = CSingleUserRecipientFilter(pRecipient);
		break;

	case MSG_UNICAST_RELIABLE:
		s_pFilterBuffer = CReliableSingleUserRecipientFilter(pRecipient);
		break;

	case MSG_BROADCAST:
		s_pFilterBuffer = CBroadcastRecipientFilter();
		break;

	case MSG_BROADCAST_RELIABLE:
		s_pFilterBuffer = CReliableBroadcastRecipientFilter();
		break;
	}

	g_pMsgBuffer = g_pEngineServer->UserMessageBegin(&s_pFilterBuffer, nMsgType, pszMessage);
}