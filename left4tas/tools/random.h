//===== Copyright (c) 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Random number generator
//
// $Workfile: $
// $NoKeywords: $
//===========================================================================//

#ifndef RANDOM_H
#define RANDOM_H

#define NTAB 32

class CRandomStream
{
public:
	CRandomStream();

	void	SetSeed( int iSeed );

	float	RandomFloat( float flMinVal = 0.0f, float flMaxVal = 1.0f );
	int		RandomInt( int iMinVal, int iMaxVal );
	float	RandomFloatExp( float flMinVal = 0.0f, float flMaxVal = 1.0f, float flExponent = 1.0f );

public:
	int		GenerateRandomNumber();

	int m_idum;
	int m_iy;
	int m_iv[NTAB];
};

extern CRandomStream random;

#endif // RANDOM_H