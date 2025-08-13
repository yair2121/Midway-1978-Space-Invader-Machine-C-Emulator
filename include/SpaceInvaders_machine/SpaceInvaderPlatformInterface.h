#pragma once

#include "MWDisplay.h"
#include "MWSound.h"
#include "MWInput.h"
#include "SpaceInvaderMachine.h"

#include "EmulateMWSpaceInvaders.h"

/// <summary>
/// Poll platform input into the KeyPress pointer.
/// </summary>
typedef void (*poll_key_presses) (KeyPress key_presses_buffer[MAX_KEY_PRESSES], MachineState* machine_state);
typedef void (*poll_system_events) (MachineState* machine_state);

/// <summary>
/// Pause machine execution for given millisecond, runs once every machine execution loop.
/// </summary>
typedef void (*delay) (uint64_t millisecond);
/// <summary>
/// Accurate time since program start in microseconds.
/// </summary>
typedef uint64_t(*microsecond_tick) ();

typedef void (*render_frame) (COLOR_FILTER frame[FRAME_HEIGHT][FRAME_WIDTH], void* context);
typedef void (*free_renderer) (void* context);

/// <summary>
/// Functions to handle input and general events from the user, such as key presses or clicking on the exit window.
/// Provides a context for the input functions to access machine state or other necessary data.
/// </summary>
typedef struct EventsFunctions {
	poll_key_presses poll_key_presses_func;
	poll_system_events poll_system_events_func; // Function to poll system events, such as general input or window events.
} EventsFunctions;

/// <summary>
/// Functions to initialize, render, and free the display renderer.
/// </summary>
typedef struct DisplayFunctions {
	//init_renderer init_renderer_func;
	render_frame render_frame_func;
	free_renderer free_renderer_func;
} DisplayFunctions;

/// <summary>
/// The machine functions that are used to control the machine's timing (for accurate emulation) and delays (for waiting between each iteration of the machine execution loop).
/// </summary>
typedef struct TimeFunctions {
	delay sleep_func;
	microsecond_tick microsecond_tick_func;
} TimeFunctions;

/// <summary>
/// Platform specific interface for sound, display, general event and input functions.
/// Additionally, a platform can provide its own needed context.
/// </summary>
typedef struct PlatformInterface {
	void* platform_context;
	play_sound_effects play_sound_effects_func;
	EventsFunctions events;
	DisplayFunctions display;
	TimeFunctions time;
} PlatformInterface;