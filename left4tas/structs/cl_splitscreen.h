//===== Copyright © 1996-2008, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef CL_SPLITSCREEN_H
#define CL_SPLITSCREEN_H
#ifdef _WIN32
#pragma once
#endif

class INetChannel;

class ISplitScreen
{
public:

	virtual bool			Init() = 0;
	virtual void			Shutdown() = 0;

	virtual bool			AddSplitScreenUser( int nSlot, int nPlayerIndex ) = 0;
	virtual bool			AddBaseUser( int nSlot, int nPlayerIndex ) = 0;
	virtual bool			RemoveSplitScreenUser( int nSlot, int nPlayerIndex ) = 0;
	virtual int				GetActiveSplitScreenPlayerSlot() = 0;
	virtual int				SetActiveSplitScreenPlayerSlot( int nSlot ) = 0;

	virtual bool			IsValidSplitScreenSlot( int nSlot ) = 0;
	virtual int				FirstValidSplitScreenSlot() = 0; // -1 == invalid
	virtual int				NextValidSplitScreenSlot( int nPreviousSlot ) = 0; // -1 == invalid

	virtual int				GetNumSplitScreenPlayers() = 0;
	virtual int				GetSplitScreenPlayerEntity( int nSlot ) = 0;
	virtual INetChannel		*GetSplitScreenPlayerNetChan( int nSlot ) = 0;

	virtual bool			IsDisconnecting( int nSlot ) = 0;
	virtual void			SetDisconnecting( int nSlot, bool bState ) = 0;

	virtual bool			SetLocalPlayerIsResolvable( char const *pchContext, int nLine, bool bResolvable ) = 0;
	virtual bool			IsLocalPlayerResolvable() = 0;
};

extern ISplitScreen *splitscreen;

#define IS_LOCAL_PLAYER_RESOLVABLE ( splitscreen->IsLocalPlayerResolvable() )

#define SET_LOCAL_PLAYER_RESOLVABLE( a, b, c ) ( splitscreen->SetLocalPlayerIsResolvable( a, b, c ) )

#define FOR_EACH_VALID_SPLITSCREEN_PLAYER( iteratorName ) \
    for ( int iteratorName = splitscreen->FirstValidSplitScreenSlot(); iteratorName != -1; iteratorName = splitscreen->NextValidSplitScreenSlot( iteratorName ) )

#define ASSERT_LOCAL_PLAYER_RESOLVABLE() Assert( splitscreen->IsLocalPlayerResolvable() );
#define GET_ACTIVE_SPLITSCREEN_SLOT() splitscreen->GetActiveSplitScreenPlayerSlot()
#define SET_ACTIVE_SPLIT_SCREEN_PLAYER_SLOT( x ) splitscreen->SetActiveSplitScreenPlayerSlot( x )
#define GET_NUM_SPLIT_SCREEN_PLAYERS() ( splitscreen->GetNumSplitScreenPlayers() )
#define IS_VALID_SPLIT_SCREEN_SLOT( i ) ( splitscreen->IsValidSplitScreenSlot( i ) )

#endif // CL_SPLITSCREEN_H