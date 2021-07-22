// Squirrel
// Left4TAS

annoying_door <- null;
annoying_door2 <- null;

L4TAS_TIMER_ROUND_RESTART <- 0;
L4TAS_TIMER_MAP_LOADED <- 1;
L4TAS_TIMER_INTRO_FINISHED <- 2;
L4TAS_TIMER_CUSTOM <- 3;

// OnTimerStart callback
function OnTimerStart( type )
{
	if ( type == L4TAS_TIMER_ROUND_RESTART )
	{
		printl( "<0> Call from L4TAS => Round started" );
	}
	else if ( type == L4TAS_TIMER_MAP_LOADED )
	{
		printl( "<1> Call from L4TAS => Map loaded" );
	}
	else if ( type == L4TAS_TIMER_INTRO_FINISHED )
	{
		printl( "<2> Call from L4TAS => Intro finished" );
	}
	else if ( type == L4TAS_TIMER_CUSTOM )
	{
		printl( "<3> Call from L4TAS => Timer started" );
		
		if ( !annoying_door )
			annoying_door = Entities.FindByClassnameWithin(null, "prop_door_rotating", Vector(-13452.000, -5264.000, -204.000), 5.0);
		
		if ( !annoying_door2 )
			annoying_door2 = Entities.FindByClassname(null, "prop_door_rotating_checkpoint");
		
		DoEntFire( "!self", "PlayerClose", "", 0.0, null, annoying_door );
		DoEntFire( "!self", "PlayerClose", "", 0.0, null, annoying_door2 );
	}
}

// OnTimerEnd callback
function OnTimerEnd()
{
	printl( "<-1> Call from L4TAS => Segment finished" );
}

printl( "Executed Left4TAS script" );