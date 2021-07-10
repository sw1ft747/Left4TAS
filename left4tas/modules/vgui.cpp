// C++
// VGUI Module

#include <Windows.h>

#include "vgui.h"
#include "utils.h"
#include "client.h"

#include "../tools/timer.h"
#include "../prop_offsets.h"
#include "../offsets.h"
#include "vtable_hook.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

static bool __INITIALIZED__ = false;
const wchar_t *g_pwcPluginVersion = NULL;

Color g_HUDColor(200, 200, 200, 255);

IPanel *g_pPanel = NULL;
IScheme *g_pScheme = NULL;
CSurface *g_pSurface = NULL;

HFont g_surfaceFont = 0;
HFont g_surfaceFont2 = 0;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

class C_BasePlayer;

extern C_BasePlayer **s_pLocalPlayer;
extern const wchar_t *g_pwcGameVersion;
extern const char *g_szPluginVersion;
extern bhop_info g_bhopInfo;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

VTABLE_HOOK(IPanel_Hook);

//-----------------------------------------------------------------------------
// Original functions
//-----------------------------------------------------------------------------

PaintTraverseFn PaintTraverse_Original = NULL;

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

void DrawHUD()
{
	int width, height;
	wchar_t buffer[768];

	int lineBreakHeight = vhud_line_break_height.GetInt();
	C_BasePlayer *pLocal = s_pLocalPlayer[0];

	g_pEngineClient->GetScreenSize(width, height);

	surface()->DrawSetTextFont(g_surfaceFont);
	surface()->DrawSetTextColor(g_HUDColor);
	
	// Game version
	if (vhud_game_version.GetBool())
	{
		surface()->DrawSetTextPos(width * vhud_game_version_x.GetFloat(), height * vhud_game_version_y.GetFloat());
		surface()->DrawPrintText(g_pwcGameVersion, wcslen(g_pwcGameVersion));
	}

	// Plugin version
	if (vhud_plugin_version.GetBool())
	{
		surface()->DrawSetTextPos(width * vhud_plugin_version_x.GetFloat(), height * vhud_plugin_version_y.GetFloat());
		surface()->DrawPrintText(g_pwcPluginVersion, wcslen(g_pwcPluginVersion));
	}

	if (pLocal)
	{
		float flSpeed = 0.f, flSpeed2D = 0.f;
		Vector vecVelocity = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecVelocity));

		if (vhud_velocity.GetBool() || vhud_speed.GetBool())
		{
			flSpeed = vecVelocity.Length();
			flSpeed2D = vecVelocity.Length2D();
		}

		// View angles
		if (vhud_angles.GetBool())
		{
			QAngle viewangles;
			g_pEngineClient->GetViewAngles(viewangles);

			int pitchWidth = static_cast<int>(width * vhud_angles_x.GetFloat());
			int pitchHeight = static_cast<int>(height * vhud_angles_y.GetFloat());

			swprintf(buffer, ARRAYSIZE(buffer), L"Pitch: %f", viewangles[PITCH]);

			surface()->DrawSetTextPos(pitchWidth, pitchHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Yaw: %f", viewangles[YAW]);

			surface()->DrawSetTextPos(pitchWidth, pitchHeight + lineBreakHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));
		}

		// Velocity
		if (vhud_velocity.GetBool())
		{
			int headerWidth = static_cast<int>(width * vhud_velocity_x.GetFloat());
			int headerHeight = static_cast<int>(height * vhud_velocity_y.GetFloat());

			swprintf(buffer, ARRAYSIZE(buffer), L"Velocity:");

			surface()->DrawSetTextPos(headerWidth, headerHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"X: %f", vecVelocity.x);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Y: %f", vecVelocity.y);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 2);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Z: %f", vecVelocity.z);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 3);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"XY: %f", flSpeed2D);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 4);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"XYZ: %f", flSpeed);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 5);
			surface()->DrawPrintText(buffer, wcslen(buffer));
		}

		// Position
		if (vhud_origin.GetBool())
		{
			Vector vecOrigin = *reinterpret_cast<Vector *>(GetOffset(pLocal, RecvPropOffsets::m_vecOrigin));

			int headerWidth = static_cast<int>(width * vhud_origin_x.GetFloat());
			int headerHeight = static_cast<int>(height * vhud_origin_y.GetFloat());

			swprintf(buffer, ARRAYSIZE(buffer), L"Origin:");

			surface()->DrawSetTextPos(headerWidth, headerHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"X: %f", vecOrigin.x);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Y: %f", vecOrigin.y);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 2);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Z: %f", vecOrigin.z);

			surface()->DrawSetTextPos(headerWidth, headerHeight + lineBreakHeight * 3);
			surface()->DrawPrintText(buffer, wcslen(buffer));
		}

		// Bunnyhop info
		if (vhud_bhop_info.GetBool())
		{
			int jumpsWidth = static_cast<int>(width * vhud_bhop_info_x.GetFloat());
			int jumpsHeight = static_cast<int>(height * vhud_bhop_info_y.GetFloat());

			swprintf(buffer, ARRAYSIZE(buffer), L"Jumps: %lu", g_bhopInfo.nJumps);

			surface()->DrawSetTextPos(jumpsWidth, jumpsHeight + lineBreakHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Speed loss: %f", g_bhopInfo.flSpeedLoss);

			surface()->DrawSetTextPos(jumpsWidth, jumpsHeight + lineBreakHeight * 2);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"Percentage: %f %%", g_bhopInfo.flPercentage);

			surface()->DrawSetTextPos(jumpsWidth, jumpsHeight + lineBreakHeight * 3);
			surface()->DrawPrintText(buffer, wcslen(buffer));
		}

		// Speed
		if (vhud_speed.GetBool())
		{
			int speedWidth = static_cast<int>(width * vhud_speed_x.GetFloat());
			int speedHeight = static_cast<int>(height * vhud_speed_y.GetFloat());

			surface()->DrawSetTextFont(g_surfaceFont2);

			swprintf(buffer, ARRAYSIZE(buffer), L"%lu", static_cast<unsigned long>(flSpeed2D));

			surface()->DrawSetTextPos(speedWidth, speedHeight);
			surface()->DrawPrintText(buffer, wcslen(buffer));

			swprintf(buffer, ARRAYSIZE(buffer), L"%lu", static_cast<unsigned long>(g_bhopInfo.flLastSpeed));

			surface()->DrawSetTextPos(speedWidth, speedHeight + vhud_line_break_height2.GetFloat());
			surface()->DrawPrintText(buffer, wcslen(buffer));
		}

		// Timer
		if (vhud_timer.GetBool())
		{
			surface()->DrawSetTextFont(g_surfaceFont2);

			if (vhud_timer_format.GetBool())
			{
				char buff[48];
				g_Timer.GetTimeInTimerFormat(buff, ARRAYSIZE(buff), NULL, false);

				const wchar_t *pwcTimer = CStringToWideCString(buff);

				surface()->DrawSetTextPos(width * vhud_timer_x.GetFloat(), height * vhud_timer_y.GetFloat());
				surface()->DrawPrintText(pwcTimer, wcslen(pwcTimer));

				delete[] pwcTimer;
			}
			else
			{
				swprintf(buffer, ARRAYSIZE(buffer), L"%.3f", g_Timer.GetTime(false));

				surface()->DrawSetTextPos(width * vhud_timer_x.GetFloat(), height * vhud_timer_y.GetFloat());
				surface()->DrawPrintText(buffer, wcslen(buffer));
			}
		}
	}
}

void __fastcall PaintTraverse_Hooked(void *thisptr, int edx, VPANEL vguiPanel, bool forceRepaint, bool allowForce)
{
	PaintTraverse_Original(thisptr, vguiPanel, forceRepaint, allowForce);

	static VPANEL panel = 0;

	if (panel == 0)
	{
		// const char *IPanel::GetName(VPANEL vguiPanel);
		const char *pszPanelName = GetVTableFunction<const char *(__thiscall *)(void *, VPANEL)>(ipanel(), Offsets::Functions::IPanel__GetName)(ipanel(), vguiPanel);

		if (!strcmp(pszPanelName, "MatSystemTopPanel"))
			panel = vguiPanel;
	}
	else if (panel == vguiPanel && vhud_enable.GetBool() && g_pEngineClient->IsInGame() && g_pEngineClient->IsConnected())
	{
		DrawHUD();
	}
}

//-----------------------------------------------------------------------------
// Init/release VGUI module
//-----------------------------------------------------------------------------

bool IsVGUIModuleInit()
{
	return __INITIALIZED__;
}

bool InitVGUIModule()
{
	const char szVGUI_Panel[] = "VGUI_Panel009";
	const char szVGUI_Scheme[] = "VGUI_Scheme010";
	const char szVGUI_Surface[] = "VGUI_Surface031";

	HMODULE vgui2_DLL = GetModuleHandle(L"vgui2.dll");
	HMODULE vguimatsurfaceDLL = GetModuleHandle(L"vguimatsurface.dll");

	ISchemeManager *pSchemeManager = NULL;

	if (vgui2_DLL)
	{
		auto vgui2Factory = (CreateInterfaceFn)GetProcAddress(vgui2_DLL, "CreateInterface");

		g_pPanel = reinterpret_cast<IPanel *>(GetInterface(vgui2Factory, szVGUI_Panel));
		pSchemeManager = reinterpret_cast<ISchemeManager *>(GetInterface(vgui2Factory, szVGUI_Scheme));
	}
	
	if (vguimatsurfaceDLL)
	{
		auto vguimatsurfaceFactory = (CreateInterfaceFn)GetProcAddress(vguimatsurfaceDLL, "CreateInterface");
		g_pSurface = reinterpret_cast<CSurface *>(GetInterface(vguimatsurfaceFactory, szVGUI_Surface));
	}

	if (!pSchemeManager)
	{
		FailedIFace("ISchemeManager");
		return false;
	}

	g_pScheme = pSchemeManager->GetIScheme(pSchemeManager->GetDefaultScheme());

	if (!g_pPanel)
	{
		FailedIFace("IPanel");
		return false;
	}
	
	if (!g_pSurface)
	{
		FailedIFace("ISurface");
		return false;
	}

	if (!g_pScheme)
	{
		FailedIFace("IScheme");
		return false;
	}

	// Init custom font
	if (!(g_surfaceFont = surface()->__CreateFont()) || !surface()->__SetFontGlyphSet(g_surfaceFont, "Lucida-Console", 21, 700, 0, 0, FONTFLAG_DROPSHADOW | FONTFLAG_OUTLINE))
	{
		Warning("[L4TAS] Failed to init custom font\n");
		return false;
	}

	if (!(g_surfaceFont2 = surface()->__CreateFont()) || !surface()->__SetFontGlyphSet(g_surfaceFont2, "Lucida-Console", 35, 700, 0, 0, FONTFLAG_DROPSHADOW | FONTFLAG_OUTLINE))
	{
		Warning("[L4TAS] Failed to init custom font #2\n");
		return false;
	}

	HOOK_VTABLE(IPanel_Hook, g_pPanel, Offsets::Functions::IPanel__PaintTraverse + 1);
	HOOK_VTABLE_FUNC(IPanel_Hook, PaintTraverse_Hooked, Offsets::Functions::IPanel__PaintTraverse, PaintTraverse_Original, PaintTraverseFn);

	g_pwcPluginVersion = CStringToWideCString(g_szPluginVersion);

	__INITIALIZED__ = true;
	return true;
}

void ReleaseVGUIModule()
{
	if (!__INITIALIZED__)
		return;

	UNHOOK_VTABLE_FUNC(IPanel_Hook, Offsets::Functions::IPanel__PaintTraverse);
	REMOVE_VTABLE_HOOK(IPanel_Hook);
}

//-----------------------------------------------------------------------------
// Console commands/variables
//-----------------------------------------------------------------------------

CON_COMMAND(vhud_setcolor, "Set color of HUD: R G B A")
{
	if (args.ArgC() < 4)
	{
		Msg("Usage: hud_setcolor [red] [green] [blue] [optional: alpha]\n");
		return;
	}

	int alpha = g_HUDColor.a();
	int red = atol(args.Arg(1));
	int green = atol(args.Arg(2));
	int blue = atol(args.Arg(3));

	red = clamp(red, 0, 255);
	green = clamp(green, 0, 255);
	blue = clamp(blue, 0, 255);
	
	if (args.ArgC() >= 5)
	{
		alpha = atol(args.Arg(4));
		alpha = clamp(alpha, 0, 255);
	}

	g_HUDColor.SetColor(red, green, blue, alpha);
}

CON_COMMAND(vhud_getcolor, "Get color of HUD: R G B A")
{
	Msg("Color: %d %d %d %d\n", g_HUDColor.r(), g_HUDColor.g(), g_HUDColor.b(), g_HUDColor.a());
}

ConVar vhud_enable("vhud_enable", "0", FCVAR_RELEASE, "Toggle HUD");
ConVar vhud_line_break_height("vhud_line_break_height", "25", FCVAR_RELEASE, "Height of line break");
ConVar vhud_line_break_height2("vhud_line_break_height2", "35", FCVAR_RELEASE, "Height of line break");

ConVar vhud_game_version("vhud_game_version", "1", FCVAR_RELEASE, "Show current version of the game");
ConVar vhud_game_version_x("vhud_game_version_x", "0.965", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_game_version_y("vhud_game_version_y", "0.01", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_plugin_version("vhud_plugin_version", "1", FCVAR_RELEASE, "Show current version of Left4TAS");
ConVar vhud_plugin_version_x("vhud_plugin_version_x", "0.007", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_plugin_version_y("vhud_plugin_version_y", "0.01", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_angles("vhud_angles", "1", FCVAR_RELEASE, "Show local player's view angles");
ConVar vhud_angles_x("vhud_angles_x", "0.03", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_angles_y("vhud_angles_y", "0.07", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_velocity("vhud_velocity", "1", FCVAR_RELEASE, "Show local player's velocity");
ConVar vhud_velocity_x("vhud_velocity_x", "0.81", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_velocity_y("vhud_velocity_y", "0.07", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_origin("vhud_origin", "1", FCVAR_RELEASE, "Show local player's position");
ConVar vhud_origin_x("vhud_origin_x", "0.81", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_origin_y("vhud_origin_y", "0.25", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_bhop_info("vhud_bhop_info", "1", FCVAR_RELEASE, "Show local player's bunnyhop info");
ConVar vhud_bhop_info_x("vhud_bhop_info_x", "0.03", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_bhop_info_y("vhud_bhop_info_y", "0.12", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_speed("vhud_speed", "1", FCVAR_RELEASE, "Show local player's 2D speed");
ConVar vhud_speed_x("vhud_speed_x", "0.488", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_speed_y("vhud_speed_y", "0.8", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_timer("vhud_timer", "1", FCVAR_RELEASE, "Show the server-side timer");
ConVar vhud_timer_x("vhud_timer_x", "0.03", FCVAR_RELEASE, "Fraction of width", true, 0.0f, true, 1.0f);
ConVar vhud_timer_y("vhud_timer_y", "0.235", FCVAR_RELEASE, "Fraction of height", true, 0.0f, true, 1.0f);

ConVar vhud_timer_format("vhud_timer_format", "0", FCVAR_RELEASE, "Show the time in timer format");