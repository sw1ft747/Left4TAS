// User Messages

#include "recipientfilter.h"
#include "../sdk.h"

enum UserMsgDest
{
	MSG_UNICAST = 0, // unreliable for a single client
	MSG_UNICAST_RELIABLE, // reliable for a single client
	MSG_BROADCAST, // unreliable for all clients
	MSG_BROADCAST_RELIABLE // reliable for all clients
};

enum UserMsgType
{
	TYPE_GEIGER = 0,
	TYPE_TRAIN,
	TYPE_HUDTEXT,
	TYPE_SAYTEXT,
	TYPE_SAYTEXT2,
	TYPE_TEXTMSG,
	TYPE_HUDMSG,
	TYPE_RESETHUD,
	TYPE_GAMETITLE,
	TYPE_ITEMPICKUP,
	TYPE_SHOWMENU,
	TYPE_SHAKE,
	TYPE_FADE,
	TYPE_VGUIMENU,
	TYPE_RUMBLE,
	TYPE_CLOSECAPTION,
	TYPE_CLOSECAPTIONDIRECT,
	TYPE_SENDAUDIO,
	TYPE_RAWAUDIO,
	TYPE_VOICEMASK,
	TYPE_REQUESTSTATE,
	TYPE_BARTIME,
	TYPE_DAMAGE,
	TYPE_RADIOTEXT,
	TYPE_HINTTEXT,
	TYPE_KEY,
	TYPE_RELOADEFFECT,
	TYPE_PLAYERANIMEVENT,
	TYPE_AMMODENIED,
	TYPE_UPDATERADAR,
	TYPE_KILLCAM,
	TYPE_MARKACHIEVEMENT,
	TYPE_SPLATTER,
	TYPE_MELEESLASHSPLATTER,
	TYPE_MELEECLUB,
	TYPE_MUDSPLATTER,
	TYPE_SPLATTERCLEAR,
	TYPE_MESSAGETEXT,
	TYPE_TRANSITIONRESTORE,
	TYPE_SPAWN,
	TYPE_CREDITSLINE,
	TYPE_CREDITSMSG,
	TYPE_JOINLATEMSG,
	TYPE_STATSCRAWLMSG,
	TYPE_STATSSKIPSTATE,
	TYPE_SHOWSTATS,
	TYPE_BLURFADE,
	TYPE_MUSICCMD,
	TYPE_WITCHBLOODSPLATTER,
	TYPE_ACHIEVEMENTEVENT,
	TYPE_PZDMGMSG,
	TYPE_ALLPLAYERSCONNECTEDGAMESTARTING,
	TYPE_VOTEREGISTERED,
	TYPE_DISCONNECTTOLOBBY,
	TYPE_CALLVOTEFAILED,
	TYPE_STEAMWEAPONSTATDATA,
	TYPE_CURRENTTIMESCALE,
	TYPE_DESIREDTIMESCALE,
	TYPE_PZENDGAMEPANELMSG,
	TYPE_PZENDGAMEPANELVOTEREGISTEREDMSG,
	TYPE_PZENDGAMEVOTESTATSMSG,
	TYPE_VOTESTART,
	TYPE_VOTEPASS,
	TYPE_VOTEFAIL
};

extern IVEngineServer *g_pEngineServer;
extern bf_write *g_pMsgBuffer;
extern const char *g_szUserMessages[];

void UserMessageBegin(UserMsgDest dest, const int nMsgType, const char *pszMessage, edict_t *pRecipient);

inline void MessageBegin(UserMsgDest dest, UserMsgType type, edict_t *pRecipient = NULL)
{
	UserMessageBegin(dest, static_cast<int>(type), g_szUserMessages[type], pRecipient);
}

inline void MessageBegin(IRecipientFilter *filter, UserMsgType type)
{
	g_pMsgBuffer = g_pEngineServer->UserMessageBegin(filter, static_cast<int>(type), g_szUserMessages[type]);
}

inline void MessageEnd()
{
	g_pEngineServer->MessageEnd();
	g_pMsgBuffer = NULL;
}

inline void WriteChar(int val)
{
	g_pMsgBuffer->WriteChar(val);
}

inline void WriteByte(int val)
{
	g_pMsgBuffer->WriteByte(val);
}

inline void WriteShort(int val)
{
	g_pMsgBuffer->WriteShort(val);
}

inline void WriteWord(int val)
{
	g_pMsgBuffer->WriteWord(val);
}

inline void WriteLong(long val)
{
	g_pMsgBuffer->WriteLong(val);
}

inline void WriteLongLong(int64 val)
{
	g_pMsgBuffer->WriteLongLong(val);
}

inline void WriteFloat(float val)
{
	g_pMsgBuffer->WriteFloat(val);
}

inline bool WriteBytes(const void *pBuf, int nBytes)
{
	return g_pMsgBuffer->WriteBytes(pBuf, nBytes);
}

inline bool WriteString(const char *pStr)
{
	return g_pMsgBuffer->WriteString(pStr);
}