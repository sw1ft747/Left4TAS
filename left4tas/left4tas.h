// C++
// Left4TAS

#pragma once

#include <Windows.h>
#include <vector>

#include "sdk.h"

#define PLUGIN_VER "1.1.0"

// Solve incorrect linking
int (WINAPIV *__vsnprintf)(char *, size_t, const char *, va_list) = _vsnprintf;
int (WINAPIV *__vsnwprintf)(wchar_t *, size_t, const wchar_t *, ...) = _snwprintf;

// Interfaces
IServer *g_pServer = NULL;
IBaseClientDLL *g_pClient = NULL;
IServerGameDLL *g_pServerGameDLL = NULL;
IServerGameClients *g_pServerGameClients = NULL;
IVEngineServer *g_pEngineServer = NULL;
IVEngineClient *g_pEngineClient = NULL;
IEngineTrace *g_pEngineTrace = NULL;
IEngineVGui *g_pEngineVGui = NULL;
IVDebugOverlay *g_pDebugOverlay = NULL;
IPlayerInfoManager *g_pPlayerInfoManager = NULL;
IGameEventManager2 *g_pGameEventManager = NULL;
IServerTools *g_pServerTools = NULL;
IServerPluginHelpers *g_pServerPluginHelpers = NULL;
IClientEntityList *g_pClientEntityList = NULL;
ICvar *g_pCVar = NULL;
IBaseFileSystem *g_pFileSystem = NULL;

CGlobalVars *gpGlobals = NULL;

const char *g_szPluginVersion = PLUGIN_VER;

class CLeft4TAS : public IServerPluginCallbacks /*, public IGameEventListener2 */
{
public:
	CLeft4TAS();

	// IServerPluginCallbacks methods
	virtual bool			Load( CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory );
	virtual void			Unload( void );
	virtual void			Pause( void );
	virtual void			UnPause( void );
	virtual const char		*GetPluginDescription( void );
	virtual void			LevelInit( char const *pMapName );
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax );
	virtual void			GameFrame( bool simulating );
	virtual void			LevelShutdown( void );
	virtual void			ClientActive( edict_t *pEntity );
	virtual void			ClientDisconnect( edict_t *pEntity );
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername );
	virtual void			SetCommandClient( int index );
	virtual void			ClientSettingsChanged( edict_t *pEdict );
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen );
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity, const CCommand &args );
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID );
	virtual void			OnQueryCvarValueFinished( QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue );

	// Version 3 of the interface
	virtual void			OnEdictAllocated( edict_t *edict );
	virtual void			OnEdictFreed( const edict_t *edict );

	/*

	// IGameEventListener2 methods
	virtual void			FireGameEvent( IGameEvent *event );
	virtual int				GetEventDebugID( void );

	*/
};