// Squirrel
// Left4TAS

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
	}
}

// OnTimerEnd callback
function OnTimerEnd()
{
	printl( "<-1> Call from L4TAS => Segment finished" );
}

printl( "Executed Left4TAS script" );