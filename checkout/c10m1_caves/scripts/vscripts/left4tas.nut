// Left4TAS

annoying_door <- null;
annoying_door2 <- null;

L4TAS_CB_ROUND_RESTART <- 0;
L4TAS_CB_MAP_LOADED <- 1;
L4TAS_CB_INTRO_FINISHED <- 2;
L4TAS_CB_SERVER_TRANSITION_FINISHED <- 3;
L4TAS_CB_TRANSITION_FINISHED <- 4;
L4TAS_CB_TIMER_STARTED <- 5;
L4TAS_CB_TIMER_STOPPED <- 6;

// Main callback function
function Left4TAS_Callbacks( type )
{
	if ( type == L4TAS_CB_ROUND_RESTART )
	{
		printl( "<0> Call from L4TAS => Round started" );
	}
	else if ( type == L4TAS_CB_MAP_LOADED )
	{
		printl( "<1> Call from L4TAS => Map loaded" );
	}
	else if ( type == L4TAS_CB_INTRO_FINISHED )
	{
		printl( "<2> Call from L4TAS => Intro finished" );
	}
	else if ( type == L4TAS_CB_SERVER_TRANSITION_FINISHED )
	{
		printl( "<3> Call from L4TAS => Server transition finished" );
	}
	else if ( type == L4TAS_CB_TRANSITION_FINISHED )
	{
		printl( "<4> Call from L4TAS => Transition finished" );
	}
	else if ( type == L4TAS_CB_TIMER_STARTED )
	{
		printl( "<5> Call from L4TAS => Timer started" );
		
		if ( !annoying_door )
			annoying_door = Entities.FindByClassnameWithin(null, "prop_door_rotating", Vector(-13452.000, -5264.000, -204.000), 10.0);
		
		if ( !annoying_door2 )
			annoying_door2 = Entities.FindByClassname(null, "prop_door_rotating_checkpoint");
		
		DoEntFire( "!self", "PlayerClose", "", 0.0, null, annoying_door );
		DoEntFire( "!self", "PlayerClose", "", 0.0, null, annoying_door2 );
	}
	else if ( type == L4TAS_CB_TIMER_STOPPED )
	{
		printl( "<6> Call from L4TAS => Timer stopped" );
	}
}

printl( "Executed Left4TAS script" );