// C++
// Left4TAS

#include "left4tas.h"

#include "patterns.h"
#include "offsets.h"
#include "signature_scanner.h"

#include "utils.h"
#include "cvars.h"

#include "modules/engine.h"
#include "modules/client.h"
#include "modules/server.h"
#include "modules/vstdlib.h"
#include "modules/vscript.h"
#include "modules/vgui.h"

#include "tools/tools.h"

#include "libdasm/libdasm.h"

#include <direct.h>
#include <string>

//-----------------------------------------------------------------------------

static bool PLUGIN_LOADED = false;

char g_szDirectory[FILENAME_MAX];
const char *g_pszDirectory = g_szDirectory;

std::string sInputsDirectory;

const char *g_pszGameVersion = NULL;
const wchar_t *g_pwcGameVersion = NULL;

DWORD g_GameVersion = 0;

//-----------------------------------------------------------------------------

CLeft4TAS g_Left4TAS;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CLeft4TAS, IServerPluginCallbacks, "ISERVERPLUGINCALLBACKS003", g_Left4TAS);

//-----------------------------------------------------------------------------

void CheckCurrentMap(const char *pszMapName)
{
	extern bool is_c5m5;
	extern bool is_c13m4;

	if (!strcmp(pszMapName, "c5m5_bridge"))
	{
		is_c5m5 = true;
		is_c13m4 = false;
	}
	else if (!strcmp(pszMapName, "c13m4_cutthroatcreek"))
	{
		is_c5m5 = false;
		is_c13m4 = true;
	}
	else
	{
		is_c13m4 = is_c5m5 = false;
	}
}

//-----------------------------------------------------------------------------

bool GetGameVersion()
{
	void *pVersion_String = LookupForString(L"engine.dll", "Version %s ");

	if (pVersion_String)
	{
		void *pVersion = FindAddress(L"engine.dll", pVersion_String);

		if (pVersion)
		{
			int shift = -1;
			char buffer[5] = { 0 };

			g_pszGameVersion = *reinterpret_cast<const char **>(GetOffset(pVersion, Offsets::Variables::spszVersionString));
			g_pwcGameVersion = CStringToWideCString(g_pszGameVersion);

			// Convert version X.X.X.X (string) to XXXX (number)
			for (int i = 0; i < 7; ++i)
			{
				if (!(i % 2))
					buffer[++shift] = g_pszGameVersion[i];
			}

			g_GameVersion = atol(buffer);
			return g_GameVersion != 0;
		}
	}

	return false;
}

void CheckGameVersion()
{
	// Change offsets/signatures for different versions

	if (g_GameVersion == 2000)
	{
		Msg("Oldest version.. Is everything working?\n");
	}
}

//-----------------------------------------------------------------------------

CLeft4TAS::CLeft4TAS()
{
}

bool CLeft4TAS::Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
{
	// Latest versions of interfaces must be used to support the old ones
	const char szClient[] = "VClient016";
	const char szServerGameDLL[] = "ServerGameDLL005";
	const char szServerGameClients[] = "ServerGameClients003";
	const char szVEngineServer[] = "VEngineServer022";
	const char szVEngineClient[] = "VEngineClient013";
	const char szEngineTrace[] = "EngineTraceServer003";
	const char szEngineVGui[] = "VEngineVGui001";
	const char szVDebugOverlay[] = "VDebugOverlay003";
	const char szPlayerInfoManager[] = "PlayerInfoManager002";
	const char szGameEventManager2[] = "GAMEEVENTSMANAGER002";
	const char szServerTools[] = "VSERVERTOOLS001";
	const char szServerPluginHelpers[] = "ISERVERPLUGINHELPERS001";
	const char szClientEntityList[] = "VClientEntityList003";
	const char szCvar[] = "VEngineCvar007";
	const char szBaseFileSystem[] = "VBaseFileSystem012";

	INSTRUCTION instruction;
	std::string sDirectory;

	if (!_getcwd(g_szDirectory, sizeof(g_szDirectory)))
	{
		Warning("[Left4TAS] Failed to get current directory lmao\n");
		return false;
	}

	sDirectory = g_szDirectory;
	sInputsDirectory = sDirectory + "\\left4dead2\\cfg\\left4tas\\";

	if (!CreateDirectoryA(sInputsDirectory.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		Warning("[Left4TAS] Failed to create ../cfg/left4tas/ directory\n");
		return false;
	}

	sInputsDirectory = sDirectory + "\\left4dead2\\cfg\\left4tas\\inputs\\";
	
	if (!CreateDirectoryA(sInputsDirectory.c_str(), NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
	{
		Warning("[Left4TAS] Failed to create ../cfg/left4tas/inputs/ directory\n");
		return false;
	}

	if (!GetGameVersion())
	{
		FailedInit("GetGameVersion");
		return false;
	}

	if (g_GameVersion < 2000 || g_GameVersion > 2155)
	{
		Warning("[Left4TAS] Game version not supported\n");
		return false;
	}

	CheckGameVersion();

	// Looking for the string used in CBaseServer::SpawnServer function
	void *pSpawnServer_String = LookupForString(L"engine.dll", "Set up players");

	if (pSpawnServer_String)
	{
		void *pSpawnServer = FindAddress(L"engine.dll", pSpawnServer_String);

		if (pSpawnServer)
		{
			get_instruction(&instruction, (BYTE *)GetOffset(pSpawnServer, Offsets::Variables::sv), MODE_32);

			if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
			{
				g_pServer = reinterpret_cast<IServer *>(instruction.op2.immediate);
				goto SERVER_SUCCESS;
			}
		}
	}

	FailedIFace("IServer");
	return false;

SERVER_SUCCESS:

	HMODULE clientDLL = GetModuleHandle(L"client.dll");

	if (clientDLL)
	{
		auto clientFactory = (CreateInterfaceFn)GetProcAddress(clientDLL, "CreateInterface");

		g_pClient = reinterpret_cast<IBaseClientDLL *>(GetInterface(clientFactory, szClient));
		g_pClientEntityList = reinterpret_cast<IClientEntityList *>(GetInterface(clientFactory, szClientEntityList));
	}

	if (!g_pClient)
	{
		FailedIFace("IBaseClientDLL");
		return false;
	}
	
	if (!g_pClientEntityList)
	{
		FailedIFace("IClientEntityList");
		return false;
	}

	HMODULE fileSystemDLL = GetModuleHandle(L"filesystem_stdio.dll");

	if (fileSystemDLL)
	{
		auto fileSystemFactory = (CreateInterfaceFn)GetProcAddress(fileSystemDLL, "CreateInterface");
		g_pFileSystem = reinterpret_cast<IBaseFileSystem *>(GetInterface(fileSystemFactory, szBaseFileSystem));
	}

	if (!g_pFileSystem)
	{
		FailedIFace("IBaseFileSystem");
		return false;
	}

	g_pServerGameDLL = reinterpret_cast<IServerGameDLL *>(GetInterface(gameServerFactory, szServerGameDLL));

	if (!g_pServerGameDLL)
	{
		FailedIFace("IServerGameDLL");
		return false;
	}
	
	g_pServerGameClients = reinterpret_cast<IServerGameClients *>(GetInterface(gameServerFactory, szServerGameClients));

	if (!g_pServerGameClients)
	{
		FailedIFace("IServerGameClients");
		return false;
	}

	g_pEngineServer = reinterpret_cast<IVEngineServer *>(GetInterface(interfaceFactory, szVEngineServer));

	if (!g_pEngineServer)
	{
		FailedIFace("IVEngineServer");
		return false;
	}
	
	g_pEngineClient = reinterpret_cast<IVEngineClient *>(GetInterface(interfaceFactory, szVEngineClient));

	if (!g_pEngineClient)
	{
		FailedIFace("IVEngineClient");
		return false;
	}
	
	g_pEngineTrace = reinterpret_cast<IEngineTrace *>(GetInterface(interfaceFactory, szEngineTrace));

	if (!g_pEngineTrace)
	{
		FailedIFace("IEngineTrace");
		return false;
	}
	
	g_pEngineVGui = reinterpret_cast<IEngineVGui *>(GetInterface(interfaceFactory, szEngineVGui));

	if (!g_pEngineVGui)
	{
		FailedIFace("IEngineVGui");
		return false;
	}
	
	g_pDebugOverlay = reinterpret_cast<IVDebugOverlay *>(GetInterface(interfaceFactory, szVDebugOverlay));

	if (!g_pDebugOverlay)
	{
		FailedIFace("IVDebugOverlay");
		return false;
	}

	g_pPlayerInfoManager = reinterpret_cast<IPlayerInfoManager *>(GetInterface(gameServerFactory, szPlayerInfoManager));

	if (!g_pPlayerInfoManager)
	{
		FailedIFace("IPlayerInfoManager");
		return false;
	}
	else
	{
		gpGlobals = g_pPlayerInfoManager->GetGlobalVars();
	}

	g_pGameEventManager = reinterpret_cast<IGameEventManager2 *>(GetInterface(interfaceFactory, szGameEventManager2));

	if (!g_pGameEventManager)
	{
		FailedIFace("IGameEventManager2");
		return false;
	}

	g_pServerTools = reinterpret_cast<IServerTools *>(GetInterface(gameServerFactory, szServerTools));

	if (!g_pServerTools)
	{
		FailedIFace("IServerTools");
		return false;
	}

	g_pServerPluginHelpers = reinterpret_cast<IServerPluginHelpers *>(GetInterface(interfaceFactory, szServerPluginHelpers));

	if (!g_pServerPluginHelpers)
	{
		FailedIFace("IServerPluginHelpers");
		return false;
	}

	g_pCVar = reinterpret_cast<ICvar *>(GetInterface(interfaceFactory, szCvar));

	if (!g_pCVar)
	{
		FailedIFace("ICvar");
		return false;
	}

	InitTools();

	if (!InitVScriptModule())
		Warning("[L4TAS] VScript functions are unavailable\n");

	if (!InitVSTDLibModule())
		Warning("[L4TAS] VSTDLib functions are unavailable\n");

	if (!InitEngineModule())
		Warning("[L4TAS] Engine functions are unavailable\n");
	
	if (!InitServerModule())
		Warning("[L4TAS] Server functions are unavailable\n");

	if (!InitClientModule())
		Warning("[L4TAS] Client functions are unavailable\n");

	if (!InitVGUIModule())
		Warning("[L4TAS] VGUI functions are unavailable\n");

	ConVar_Register();
	
	g_pEngineServer->ServerCommand("exec left4tas/tas_plugin_load\n");

	ConColorMsg({ g_bFailedInit ? 255 : 0, 255, 0, 255 }, g_bFailedInit ? "[Left4TAS] Loaded with limited features\n" : "[Left4TAS] Successfully loaded\n");

	PLUGIN_LOADED = true;
	return true;
}

void CLeft4TAS::Unload(void)
{
	ReleaseVScriptModule();
	ReleaseVSTDLibModule();
	ReleaseEngineModule();
	ReleaseServerModule();
	ReleaseClientModule();
	ReleaseVGUIModule();

	ReleaseTools();

	ConVar_Unregister();

	delete[] g_pwcGameVersion;

	if (PLUGIN_LOADED)
		Msg("[Left4TAS] Successfully unloaded\n");
}

const char *CLeft4TAS::GetPluginDescription(void)
{
	return "Left4TAS v" PLUGIN_VER " : Sw1ft";
}

/*
void CLeft4TAS::FireGameEvent(IGameEvent *event)
{
	const char *pszName = event->GetName();
}

int CLeft4TAS::GetEventDebugID()
{
	return EVENT_DEBUG_ID_INIT;
}
*/

void CLeft4TAS::Pause(void)
{
}

void CLeft4TAS::UnPause(void)
{
}

void CLeft4TAS::GameFrame(bool simulating)
{
}

void CLeft4TAS::LevelInit(char const *pMapName)
{
	CheckCurrentMap(pMapName);
}

void CLeft4TAS::ServerActivate(edict_t *pEdictList, int edictCount, int clientMax)
{
}

void CLeft4TAS::LevelShutdown(void)
{
}

void CLeft4TAS::ClientActive(edict_t *pEntity)
{
}

void CLeft4TAS::ClientDisconnect(edict_t *pEntity)
{
}

void CLeft4TAS::ClientPutInServer(edict_t *pEntity, char const *playername)
{
}

void CLeft4TAS::SetCommandClient(int index)
{
}

void CLeft4TAS::ClientSettingsChanged(edict_t *pEdict)
{
}

PLUGIN_RESULT CLeft4TAS::ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
{
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CLeft4TAS::ClientCommand(edict_t *pEntity, const CCommand &args)
{
	return PLUGIN_CONTINUE;
}

PLUGIN_RESULT CLeft4TAS::NetworkIDValidated(const char *pszUserName, const char *pszNetworkID)
{
	return PLUGIN_CONTINUE;
}

void CLeft4TAS::OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue)
{
}

void CLeft4TAS::OnEdictAllocated(edict_t *edict)
{
}

void CLeft4TAS::OnEdictFreed(const edict_t *edict)
{
}