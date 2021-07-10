// C++
// Console Variables

#pragma once

#pragma warning(disable : 4005)

#include "sdk.h"

extern ConVar wait_frames_pause;

extern ConVar category_no_director;
extern ConVar category_no_survivor_bots;

extern ConVar tas_autojump;
extern ConVar tas_timescale;

 // Strafing stuff
extern ConVar tas_strafe;

extern ConVar tas_strafe_autojump;
extern ConVar tas_strafe_yaw;
extern ConVar tas_strafe_buttons;

extern ConVar tas_strafe_vectorial;
extern ConVar tas_strafe_vectorial_increment;
extern ConVar tas_strafe_vectorial_offset;
extern ConVar tas_strafe_vectorial_snap;

extern ConVar tas_strafe_ignore_ground;
extern ConVar tas_strafe_tickrate;

extern ConVar tas_force_wishspeed_cap;

// Angles stuff
extern ConVar tas_setpitch;
extern ConVar tas_setyaw;
extern ConVar tas_setanglespeed;

// HUD
extern ConVar vhud_enable;
extern ConVar vhud_line_break_height;
extern ConVar vhud_line_break_height2;

extern ConVar vhud_game_version;
extern ConVar vhud_game_version_x;
extern ConVar vhud_game_version_y;

extern ConVar vhud_plugin_version;
extern ConVar vhud_plugin_version_x;
extern ConVar vhud_plugin_version_y;

extern ConVar vhud_angles;
extern ConVar vhud_angles_x;
extern ConVar vhud_angles_y;

extern ConVar vhud_velocity;
extern ConVar vhud_velocity_x;
extern ConVar vhud_velocity_y;

extern ConVar vhud_origin;
extern ConVar vhud_origin_x;
extern ConVar vhud_origin_y;

extern ConVar vhud_bhop_info;
extern ConVar vhud_bhop_info_x;
extern ConVar vhud_bhop_info_y;

extern ConVar vhud_speed;
extern ConVar vhud_speed_x;
extern ConVar vhud_speed_y;

extern ConVar vhud_timer_format;
extern ConVar vhud_timer;
extern ConVar vhud_timer_x;
extern ConVar vhud_timer_y;

// Timer
extern ConVar timer_auto;

// Pause
extern ConVar prevent_pause;
extern ConVar prevent_unpause;

// Files execution
extern ConVar tas_autoexec_configs;
extern ConVar tas_autorun_vscripts;
extern ConVar tas_timer_callbacks;