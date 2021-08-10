//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose:
//
// $NoKeywords: $
//===========================================================================//

#ifndef CMD_H
#define CMD_H
#ifdef _WIN32
#pragma once
#endif

typedef enum 
{
	CBUF_FIRST_PLAYER = 0,
	CBUF_LAST_PLAYER = 1,
} ECommandTarget_t;

enum cmd_source_t
{
	// Added to the console buffer by gameplay code.  Generally unrestricted.
	kCommandSrcCode,

	// Sent from code via engine->ClientCmd, which is restricted to commands visible
	// via FCVAR_CLIENTCMD_CAN_EXECUTE.
	kCommandSrcClientCmd,

	// Typed in at the console or via a user key-bind.  Generally unrestricted, although
	// the client will throttle commands sent to the server this way to 16 per second.
	kCommandSrcUserInput,

	// Came in over a net connection as a clc_stringcmd
	// host_client will be valid during this state.
	//
	// Restricted to FCVAR_GAMEDLL commands (but not convars) and special non-ConCommand
	// server commands hardcoded into gameplay code (e.g. "joingame")
	kCommandSrcNetClient,

	// Received from the server as the client
	//
	// Restricted to commands with FCVAR_SERVER_CAN_EXECUTE
	kCommandSrcNetServer,

	// Being played back from a demo file
	//
	// Not currently restricted by convar flag, but some commands manually ignore calls
	// from this source.  FIXME: Should be heavily restricted as demo commands can come
	// from untrusted sources.
	kCommandSrcDemoFile,

	// Invalid value used when cleared
	kCommandSrcInvalid = -1
};

typedef void (*Cbuf_AddTextFn)(ECommandTarget_t eTarget, const char *text, cmd_source_t source);
typedef void (*Cbuf_ExecuteFn)();

#endif // CMD_H