//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef INPUTDATA_T_H
#define INPUTDATA_T_H
#ifdef _WIN32
#pragma once
#endif

#include "../game/server/variant_t.h"

struct inputdata_t
{
	CBaseEntity *pActivator;		// The entity that initially caused this chain of output events.
	CBaseEntity *pCaller;			// The entity that fired this particular output.
	variant_t value;				// The data parameter for this output.
	int nOutputID;					// The unique ID of the output that was fired.
};

#endif // INPUTDATA_T_H