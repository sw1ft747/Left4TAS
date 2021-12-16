//===== Copyright © 1996-2009, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef FUNCTORS_L4D2_H
#define FUNCTORS_L4D2_H
#ifdef _WIN32
#pragma once
#endif

class CTerrorPlayer;

enum SurvivorCharacterType
{
	Nick = 0,
	Rochelle,
	Coach,
	Ellis
};

struct FindCharacter
{
	SurvivorCharacterType characterType;
	CTerrorPlayer *player;
};

#endif // FUNCTORS_L4D2_H