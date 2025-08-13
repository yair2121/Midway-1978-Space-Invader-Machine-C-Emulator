#pragma once

#include "Emulate8080.h"

#include "SpaceInvaderPlatformInterface.h"

#define SP_START 0x2400
#define PATH_BUFFER_SIZE 0x100
#define INVADERS_RAM_SIZE 0x4000 // 16KB of RAM

typedef struct GameRom {
	size_t size;
	uint8_t* code_buffer;
} GameRom;

typedef struct MachineState {
	Cpu8080* cpu;
	MWState* mwState;
	PlatformInterface platform_interface;
	bool previous_active_sound_effects[NUMBER_OF_SOUND_EFFECTS]; // Sound effects should only be played when set, UFO is persistent
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
MachineState* init_machine(char rom_path[PATH_BUFFER_SIZE], PlatformInterface platform_interface);

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
