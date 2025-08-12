#pragma once

#include "Emulate8080.h"
#include "EmulateMWSpaceInvaders.h"
#include "MWDisplay.h"
#include "MWSound.h"

#define SP_START 0x2400

typedef void (*poll_key_presses) (KeyPress* key_presses_buffer, void* context);

typedef void (*delay) (uint64_t millisecond);
typedef uint64_t(*microsecond_tick) ();


typedef void (*play_sound_effects) (bool sound_effects_to_play[NUMBER_OF_SOUND_EFFECTS], void* platform_context);

/// <summary>
/// Functions to handle input from the user, such as key presses.
/// Provides a context for the input functions to access machine state or other necessary data.
/// </summary>
typedef struct InputFunctions {
	poll_key_presses poll_key_presses_func;
} InputFunctions;

/// <summary>
/// The machine functions that are used to control the machine's timing (for accurate emulation) and delays (for waiting between each iteration of the machine execution loop).
/// </summary>
typedef struct TimeFunctions {
	delay sleep_func;
	microsecond_tick microsecond_tick_func;
} TimeFunctions;


typedef struct MachineState {
	Cpu8080* cpu;
	MWState* mwState;
	bool previous_active_sound_effects[NUMBER_OF_SOUND_EFFECTS]; // Sound effects should only be played when set, UFO is persistent
	void* platform_context;
	play_sound_effects play_sound_effects_func; // Function to play sound effects, can be null if no sound is needed.
	DisplayFunctions display_functions;
	InputFunctions input_functions;
	TimeFunctions time_functions;
	bool is_running;
	bool should_exit; // Used to signal the machine to stop running, e.g., when the user closes the window or presses a quit key.
} MachineState;

typedef struct GAME_ROM {
	size_t size;
	uint8_t* code_buffer;
} GAME_ROM;


MachineState* init_machine(GAME_ROM game_rom, size_t memory_size, void* platform_context, play_sound_effects play_sound_effects_func, InputFunctions input_functions, DisplayFunctions display_functions, TimeFunctions time_functions);

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
