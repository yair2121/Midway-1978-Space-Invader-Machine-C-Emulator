#pragma once

#include "Emulate8080.h"
#include "EmulateMWSpaceInvaders.h"
#include "MWDisplay.h"
#include "MWSound.h"

#define SP_START 0x2400
#define PATH_BUFFER_SIZE 0x100
#define INVADERS_RAM_SIZE 0x4000 // 16KB of RAM

typedef void (*poll_key_presses) (KeyPress* key_presses_buffer, void* platform_context);

typedef void (*delay) (uint64_t millisecond);
typedef uint64_t(*microsecond_tick) ();




/// <summary>
/// The machine functions that are used to control the machine's timing (for accurate emulation) and delays (for waiting between each iteration of the machine execution loop).
/// </summary>
typedef struct TimeFunctions {
	delay sleep_func;
	microsecond_tick microsecond_tick_func;
} TimeFunctions;

typedef struct GameRom {
	size_t size;
	uint8_t* code_buffer;
} GameRom;
typedef void (*poll_system_events) (MachineState* machine_state);


/// <summary>
/// Functions to handle input and general events from the user, such as key presses or clicking on the exit window.
/// Provides a context for the input functions to access machine state or other necessary data.
/// </summary>
typedef struct EventsFunctions {
	poll_key_presses poll_key_presses_func;
	poll_system_events poll_system_events_func; // Function to poll system events, such as general input or window events.
} EventsFunctions;

typedef struct MachineState {
	Cpu8080* cpu;
	MWState* mwState;
	bool previous_active_sound_effects[NUMBER_OF_SOUND_EFFECTS]; // Sound effects should only be played when set, UFO is persistent
	void* platform_context;
	play_sound_effects play_sound_effects_func; // Function to play sound effects, can be null if no sound is needed.
	DisplayFunctions display_functions;
	EventsFunctions events_functions;
	TimeFunctions time_functions;
	bool is_running;
	bool should_exit; // Used to signal the machine to stop running, e.g., when the user closes the window or presses a quit key.
} MachineState;



/// <summary>
/// 
/// </summary>
/// <param name="rom_path">You need to provide your own 1978 Midway Space Invader arcade machine rom</param>
/// <param name="memory_size">How much ram does the cpu will have, for space invader - 0x10000 is good</param>
/// <param name="platform_context">Specific platform context, passed to Audio/Display/Input callback functions</param>
/// <param name="play_sound_effects_func">Platform specific implementation for playing a sound effect</param>
/// <param name="events_functions">Platform specific implementation for capturing input</param>
/// <param name="display_functions">Platform specific implementation for displaying the game graphics</param>
/// <param name="time_functions">Platform specific implementation for accurate timing functions</param>
/// <returns></returns>
MachineState* init_machine(char rom_path[PATH_BUFFER_SIZE], void* platform_context, play_sound_effects play_sound_effects_func, EventsFunctions events_functions, DisplayFunctions display_functions, TimeFunctions time_functions);

void free_machine(MachineState* machine_state);

/// <summary>
/// Start machine execution loop.
/// Can be paused by toggling the running state.
/// Can be exited by calling exit_machine.
/// </summary>
/// <param name="machine_state"></param>
void run_machine(MachineState* machine_state);

bool toggle_machine_running(MachineState* machine_state);

/// <summary>
/// Stop machine execution loop.
/// </summary>
/// <param name="machine_state"></param>
/// <returns></returns>
void exit_machine(MachineState* machine_state);
