// C++
// VGUI Module

#pragma once

#include "../sdk.h"

#include "../cvars.h"

#include "vgui/IPanel.h"
#include "vgui/IScheme.h"
#include "vgui/ISurface.h"

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

namespace vgui
{

enum SurfaceFeature_e
{
	ANTIALIASED_FONTS = 1,
	DROPSHADOW_FONTS = 2,
	ESCAPE_KEY = 3,
	OPENING_NEW_HTML_WINDOWS = 4,
	FRAME_MINIMIZE_MAXIMIZE = 5,
	OUTLINE_FONTS = 6,
	DIRECT_HWND_RENDER = 7,
};

enum EFontFlags
{
	FONTFLAG_NONE,
	FONTFLAG_ITALIC = 0x001,
	FONTFLAG_UNDERLINE = 0x002,
	FONTFLAG_STRIKEOUT = 0x004,
	FONTFLAG_SYMBOL = 0x008,
	FONTFLAG_ANTIALIAS = 0x010,
	FONTFLAG_GAUSSIANBLUR = 0x020,
	FONTFLAG_ROTARY = 0x040,
	FONTFLAG_DROPSHADOW = 0x080,
	FONTFLAG_ADDITIVE = 0x100,
	FONTFLAG_OUTLINE = 0x200,
	FONTFLAG_CUSTOM = 0x400,		// custom generated font - never fall back to asian compatibility mode
	FONTFLAG_BITMAP = 0x800,		// compiled bitmap font - no fallbacks
};

//-----------------------------------------------------------------------------
// Fix SDK: function ISurface::CreateNewTextureID must be defined if
// the platform is XBox
//-----------------------------------------------------------------------------

class CSurface : public ISurface
{
public:
	inline HFont __CreateFont()
	{
		static DWORD *pVTable = NULL;

		if (!pVTable)
			pVTable = *reinterpret_cast<DWORD **>(this);

		// HFont ISurface::CreateFont();
		return reinterpret_cast<HFont (__thiscall *)(void *)>(pVTable[63])(this);
	}

	inline bool __SetFontGlyphSet(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0)
	{
		static DWORD *pVTable = NULL;

		if (!pVTable)
			pVTable = *reinterpret_cast<DWORD **>(this);

		// bool ISurface::SetFontGlyphSet(HFont font, const char *windowsFontName, int tall, int weight, int blur, int scanlines, int flags, int nRangeMin = 0, int nRangeMax = 0);
		return reinterpret_cast<bool (__thiscall *)(void *, HFont, const char *, int, int, int, int, int, int, int)>(pVTable[64])(this, font, windowsFontName, tall, weight, blur, scanlines, flags, nRangeMin, nRangeMax);
	}

	inline int __GetFontTall(HFont font)
	{
		static DWORD *pVTable = NULL;

		if (!pVTable)
			pVTable = *reinterpret_cast<DWORD **>(this);

		// int ISurface::GetFontTall(HFont font);
		return reinterpret_cast<int (__thiscall *)(void *, HFont)>(pVTable[66])(this, font);
	}
};

}

using namespace vgui;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

typedef void (__thiscall *StartDrawingFn)(void *);
typedef void (__thiscall *FinishDrawingFn)(void *);

//-----------------------------------------------------------------------------
// Interfaces
//-----------------------------------------------------------------------------

extern IPanel *g_pPanel;
extern IScheme *g_pScheme;
extern CSurface *g_pSurface;

extern IVEngineClient *g_pEngineClient;

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------

void DrawHUD();

inline IPanel *ipanel()
{
	return g_pPanel;
}

inline IScheme *scheme()
{
	return g_pScheme;
}

inline CSurface *surface()
{
	return g_pSurface;
}

//-----------------------------------------------------------------------------
// Controls
//-----------------------------------------------------------------------------

bool IsVGUIModuleInit();

bool InitVGUIModule();

void ReleaseVGUIModule();