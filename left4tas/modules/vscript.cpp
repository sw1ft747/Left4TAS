// C++
// VScript Module

#include "../patterns.h"
#include "../offsets.h"
#include "../tools/misc.h"
#include "../tools/timer.h"

#include "trampoline_hook.h"
#include "signature_scanner.h"
#include "utils.h"

#include "vscript.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
static IScriptVM **s_ScriptVM = NULL;

IScriptVM *g_pScriptVM = NULL;
IScriptManager *g_pScriptManager = NULL;

HSCRIPT g_hScriptL4TAS = NULL;

//-----------------------------------------------------------------------------
// Init hooks
//-----------------------------------------------------------------------------

TRAMPOLINE_HOOK(VScriptInit_Hook);
TRAMPOLINE_HOOK(VScriptTerm_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

VScriptServerInitFn VScriptServerInit_Original = NULL;
VScriptServerTermFn VScriptServerTerm_Original = NULL;

//-----------------------------------------------------------------------------
// Class for interacting with the plugin from VScripts
//-----------------------------------------------------------------------------

class CScriptLeft4TAS
{
public:
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

	const char *GetVersion()
	{
		extern const char *g_szPluginVersion;
		return g_szPluginVersion;
	}
} g_ScriptLeft4TAS;

BEGIN_SCRIPTDESC_ROOT_NAMED(CScriptLeft4TAS, "CLeft4TAS", SCRIPT_SINGLETON "Interface for interacting with Left4TAS")
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
DEFINE_SCRIPTFUNC(GetVersion, "Get current version of Left4TAS")
END_SCRIPTDESC();

//-----------------------------------------------------------------------------
// Called when the VM started successfully
//-----------------------------------------------------------------------------

static void InitVScriptBridge()
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

static void TermVScriptBridge()
{
	if (g_pScriptVM && g_hScriptL4TAS)
	{
		g_pScriptVM->RemoveInstance(g_hScriptL4TAS);

		g_hScriptL4TAS = NULL;
		g_pScriptVM = NULL;
	}
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

bool VScriptServerInit_Hooked()
{
	bool bVMCreated = VScriptServerInit_Original();

	if (bVMCreated)
	{
		g_pScriptVM = *s_ScriptVM;
		InitVScriptBridge();
	}

	return bVMCreated;
}

void VScriptServerTerm_Hooked()
{
	TermVScriptBridge();

	VScriptServerTerm_Original();
}

//-----------------------------------------------------------------------------
// Init/release VScript module
//-----------------------------------------------------------------------------

bool IsVScriptModuleInit()
{
	return __INITIALIZED__;
}

bool InitVScriptModule()
{
	void *pVScriptServerInit = FIND_PATTERN(L"server.dll", Patterns::Server::VScriptServerInit);

	if (!pVScriptServerInit)
	{
		FailedInit("InitVScriptSystem");
		return false;
	}

	void *pVScriptServerTerm = FIND_PATTERN(L"server.dll", Patterns::Server::VScriptServerTerm);

	if (!pVScriptServerTerm)
	{
		FailedInit("VScriptServerTerm");
		return false;
	}

	if (!__UTIL_PlayerByIndex || !__GoAwayFromKeyboard || !__TakeOverBot)
		return false;

	g_pScriptManager = **reinterpret_cast<IScriptManager ***>(GetOffset(pVScriptServerTerm, Offsets::Variables::g_pScriptManager));
	s_ScriptVM = *reinterpret_cast<IScriptVM ***>(GetOffset(pVScriptServerTerm, Offsets::Variables::g_pScriptVM));

	g_pScriptVM = *s_ScriptVM;

	INIT_HOOK(VScriptInit_Hook, pVScriptServerInit, VScriptServerInit_Hooked);
	INIT_HOOK(VScriptTerm_Hook, pVScriptServerTerm, VScriptServerTerm_Hooked);

	HOOK_FUNC(VScriptInit_Hook, VScriptServerInit_Original, VScriptServerInitFn);
	HOOK_FUNC(VScriptTerm_Hook, VScriptServerTerm_Original, VScriptServerTermFn);

	if (g_pScriptVM) // already on the map, connect the plugin with VScripts
		InitVScriptBridge();

	__INITIALIZED__ = true;
	return true;
}

void ReleaseVScriptModule()
{
	if (!__INITIALIZED__)
		return;

	TermVScriptBridge();

	UNHOOK_FUNC(VScriptInit_Hook);
	UNHOOK_FUNC(VScriptTerm_Hook);

	REMOVE_HOOK(VScriptInit_Hook);
	REMOVE_HOOK(VScriptTerm_Hook);
}