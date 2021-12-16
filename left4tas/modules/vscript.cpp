// VScript Module

#include "../patterns.h"
#include "../offsets.h"
#include "../tools/misc.h"
#include "../tools/timer.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "utils.h"

#include "vscript.h"

#include "../libdasm/libdasm.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef bool (*VScriptServerInitFn)();
typedef void (*VScriptServerTermFn)();

//-----------------------------------------------------------------------------
// Imports
//-----------------------------------------------------------------------------

extern IVEngineServer *g_pEngineServer;
extern IServerPluginHelpers *g_pServerPluginHelpers;

//-----------------------------------------------------------------------------
// Global Vars
//-----------------------------------------------------------------------------

CVScript g_VScript;

IScriptVM *g_pScriptVM = NULL;
IScriptManager *g_pScriptManager = NULL;

HSCRIPT g_hScriptL4TAS = NULL;

//-----------------------------------------------------------------------------
// Declare hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(VScriptInit_Hook);
TRAMPOLINE_HOOK(VScriptTerm_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

VScriptServerInitFn VScriptServerInit_Original = NULL;
VScriptServerTermFn VScriptServerTerm_Original = NULL;

//-----------------------------------------------------------------------------
// Class for interacting with Left4TAS from VScripts
//-----------------------------------------------------------------------------

class CScriptLeft4TAS
{
public:
	void ServerCommand(const char *pszCommand)
	{
		g_pEngineServer->ServerCommand(pszCommand);
		g_pEngineServer->ServerExecute();
	}

	void ClientCommand(int index, const char *pszCommand)
	{
		edict_t *pEdict = EntIndexToEdict(index);

		if (IsEdictValid(pEdict))
			g_pServerPluginHelpers->ClientCommand(pEdict, pszCommand);
	}

	void StartTimer()
	{
		g_Timer.Start();
	}

	void StopTimer()
	{
		g_Timer.Stop();
	}

	void ResetTimer()
	{
		g_Timer.Reset();
	}

	bool IsTimerStarted()
	{
		return g_Timer.IsStarted();
	}

	void PrintTicks()
	{
		g_Timer.PrintTicks();
	}

	void PrintTime()
	{
		g_Timer.PrintTime();
	}

	float GetTime(bool bPreciseTime)
	{
		return g_Timer.GetTime(bPreciseTime);
	}

	bool SetSegmentTime(float flSegmentTime, float flSegmentPreciseTime)
	{
		return g_Timer.SetSegmentTime(flSegmentTime, flSegmentPreciseTime);
	}

	void SayText(int index, const char *pszMessage)
	{
		::SayText(index, pszMessage);
	}

	void SayTextAll(const char *pszMessage)
	{
		::SayText(-1, pszMessage);
	}

	void ClientPrint(int index, int dest, const char *pszMessage)
	{
		::ClientPrint(index, dest, pszMessage);
	}

	void ClientPrintAll(int dest, const char *pszMessage)
	{
		::ClientPrint(-1, dest, pszMessage);
	}

	void SetName(int index, const char *pszName)
	{
		::SetName(index, pszName);
	}

	void GoAwayFromKeyboard(int index)
	{
		CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(index);

		if (pPlayer)
			::GoAwayFromKeyboard(pPlayer);
	}

	bool TakeOverBot(int index)
	{
		CTerrorPlayer *pPlayer = (CTerrorPlayer *)UTIL_PlayerByIndex(index);

		if (pPlayer)
			return ::TakeOverBot(pPlayer, true);

		return false;
	}

	int CreateFakeClient()
	{
		return ::CreateFakeClient();
	}

	int CreateFakeClientNamed(const char *pszName)
	{
		return ::CreateFakeClientNamed(pszName);
	}

	int CreateFakeClientTeam(int nTeam)
	{
		return ::CreateFakeClientTeam(nTeam);
	}

	int CreateFakeClientNamedTeam(const char *pszName, int nTeam)
	{
		return ::CreateFakeClientNamedTeam(pszName, nTeam);
	}

	const char *GetCurrentMap()
	{
		return gpGlobals->mapname.ToCStr();
	}

	const char *GetVersion()
	{
		extern const char *g_szPluginVersion;
		return g_szPluginVersion;
	}
} g_ScriptLeft4TAS;

BEGIN_SCRIPTDESC_ROOT_NAMED(CScriptLeft4TAS, "CLeft4TAS", SCRIPT_SINGLETON "Interface for interacting with Left4TAS")
	DEFINE_SCRIPTFUNC(ServerCommand, "Immediately execute a server command")
	DEFINE_SCRIPTFUNC(ClientCommand, "Send a command to client")
	DEFINE_SCRIPTFUNC(StartTimer, "Start the timer")
	DEFINE_SCRIPTFUNC(StopTimer, "Stop the timer")
	DEFINE_SCRIPTFUNC(ResetTimer, "Reset the timer")
	DEFINE_SCRIPTFUNC(IsTimerStarted, "Returns true if the timer is started")
	DEFINE_SCRIPTFUNC(PrintTicks, "Print the current/total ticks")
	DEFINE_SCRIPTFUNC(PrintTime, "Print the current/total time")
	DEFINE_SCRIPTFUNC(GetTime, "Get the current/total time")
	DEFINE_SCRIPTFUNC(SetSegmentTime, "Set the time from previous segment")
	DEFINE_SCRIPTFUNC(SayText, "Print message to the client")
	DEFINE_SCRIPTFUNC(SayTextAll, "Print message to all clients")
	DEFINE_SCRIPTFUNC(ClientPrint, "Print message to the client")
	DEFINE_SCRIPTFUNC(ClientPrintAll, "Print message to all clients")
	DEFINE_SCRIPTFUNC(SetName, "Set client's name")
	DEFINE_SCRIPTFUNC(GoAwayFromKeyboard, "Go to IDLE mode")
	DEFINE_SCRIPTFUNC(TakeOverBot, "Take over a bot")
	DEFINE_SCRIPTFUNC(CreateFakeClient, "Create a fake client")
	DEFINE_SCRIPTFUNC(CreateFakeClientNamed, "Create a fake client with a given name")
	DEFINE_SCRIPTFUNC(CreateFakeClientTeam, "Create a fake client and change team to the chosen one")
	DEFINE_SCRIPTFUNC(CreateFakeClientNamedTeam, "Create a fake client with a given name and change team to the chosen one")
	DEFINE_SCRIPTFUNC(GetCurrentMap, "Get map name")
	DEFINE_SCRIPTFUNC(GetVersion, "Get current version of Left4TAS")
END_SCRIPTDESC();

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

bool VScriptServerInit_Hooked()
{
	bool bVMCreated = VScriptServerInit_Original();

	if (bVMCreated)
	{
		g_pScriptVM = *g_VScript.GetScriptVMPointer();
		g_VScript.InitVScriptBridge();
	}

	return bVMCreated;
}

void VScriptServerTerm_Hooked()
{
	g_VScript.TermVScriptBridge();

	VScriptServerTerm_Original();
}

//-----------------------------------------------------------------------------
// Called when the VM started successfully
//-----------------------------------------------------------------------------

void CVScript::InitVScriptBridge()
{
	if (g_pScriptVM)
	{
		g_pScriptVM->RegisterClass(GetScriptDescForClass(CScriptLeft4TAS));
		g_hScriptL4TAS = g_pScriptVM->RegisterInstance(&g_ScriptLeft4TAS, "Left4TAS");
	}
}

//-----------------------------------------------------------------------------
// Called before the VM will be destroyed
//-----------------------------------------------------------------------------

void CVScript::TermVScriptBridge()
{
	if (g_pScriptVM && g_hScriptL4TAS)
	{
		g_pScriptVM->RemoveInstance(g_hScriptL4TAS);

		g_hScriptL4TAS = NULL;
		g_pScriptVM = NULL;
	}
}

//-----------------------------------------------------------------------------
// VScript module implementations
//-----------------------------------------------------------------------------

CVScript::CVScript() : m_bInitialized(false), m_ppScriptVM(NULL)
{
}

bool CVScript::IsInitialized() const
{
	return m_bInitialized;
}

bool CVScript::Init()
{
	INSTRUCTION instruction;

	if (!UTIL_PlayerByIndex || !GoAwayFromKeyboard || !TakeOverBot)
		return false;

	void *pVScriptServerInit = FIND_PATTERN(L"server.dll", Patterns::Server::VScriptServerInit);

	if (!pVScriptServerInit)
	{
		FailedInit("VScriptServerInit");
		return false;
	}

	void *pVScriptServerTerm = FIND_PATTERN(L"server.dll", Patterns::Server::VScriptServerTerm);

	if (!pVScriptServerTerm)
	{
		FailedInit("VScriptServerTerm");
		return false;
	}

	// g_pScriptManager
	get_instruction(&instruction, (BYTE *)GetOffset(pVScriptServerTerm, Offsets::Variables::g_pScriptManager), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_MOV && instruction.op1.type == OPERAND_TYPE_REGISTER && instruction.op2.type == OPERAND_TYPE_MEMORY)
	{
		g_pScriptManager = *reinterpret_cast<IScriptManager **>(instruction.op2.displacement);
	}
	else
	{
		FailedInit("g_pScriptManager");
		return false;
	}

	// g_pScriptVM
	get_instruction(&instruction, (BYTE *)GetOffset(pVScriptServerTerm, Offsets::Variables::g_pScriptVM), MODE_32);

	if (instruction.type == INSTRUCTION_TYPE_CMP && instruction.op1.type == OPERAND_TYPE_MEMORY && instruction.op2.type == OPERAND_TYPE_IMMEDIATE)
	{
		m_ppScriptVM = reinterpret_cast<IScriptVM **>(instruction.op1.displacement);
	}
	else
	{
		FailedInit("g_pScriptVM");
		return false;
	}

	g_pScriptVM = *m_ppScriptVM;

	HOOK_FUNCTION(VScriptInit_Hook, pVScriptServerInit, VScriptServerInit_Hooked, VScriptServerInit_Original, VScriptServerInitFn);
	HOOK_FUNCTION(VScriptTerm_Hook, pVScriptServerTerm, VScriptServerTerm_Hooked, VScriptServerTerm_Original, VScriptServerTermFn);

	if (g_pScriptVM) // already on the map, connect the plugin with VScripts
		InitVScriptBridge();

	m_bInitialized = true;
	return true;
}

bool CVScript::Release()
{
	if (!m_bInitialized)
		return false;

	TermVScriptBridge();

	UNHOOK_FUNCTION(VScriptInit_Hook);
	UNHOOK_FUNCTION(VScriptTerm_Hook);

	return true;
}