#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDL_platform.h"

static bool init_sdl_context(SDL_CONTEXT* sdl_context, char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100]) {
	if (init_input_sdl() == false) {
		return 1;
	}
	if (init_sound_effects_sdl(&sdl_context->sound_effects, sound_effect_paths) == false) {
		destroy_input_sdl();
		return 1;
	}

	if (init_renderer_sdl(&sdl_context->display) == false) {
		destroy_sound_effects_sdl(&sdl_context->sound_effects);
		destroy_input_sdl();
		return 1;
	}
	return true;
}

int main(int argc, char* argv[]) {
	char* game_rom_path = argv[1];
	char* sound_directory = argv[2];

	char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100];
	for (int i = 0; i < NUMBER_OF_SOUND_EFFECTS; i++) {
		sprintf_s(sound_effect_paths[i], sizeof(sound_effect_paths[i]), "%s%d.wav", sound_directory, i);
	}
	SDL_CONTEXT sdl_context;

	init_sdl_context(&sdl_context, sound_effect_paths);

	DisplayFunctions display_functions = (DisplayFunctions) { render_frame_SDL};
	EventsFunctions events_functions = (EventsFunctions) { poll_keys, sdl_handle_system_events };
	TimeFunctions time_functions = (TimeFunctions) { SDL_Delay, get_microseconds_since_start };
	SoundFunctions sound_functions = (SoundFunctions){ sdl_play_sound_effects};
	PlatformInterface platform_interface = {
		.platform_context = &sdl_context,
		.sound = sound_functions,
		.events = events_functions,
		.display = display_functions,
		.time = time_functions
	};
	MachineState* machine_state = init_machine(game_rom_path, platform_interface); // 16K
	if (machine_state == NULL) {
		destroy_renderer_sdl(&sdl_context);
		SDL_Log("Failed to initialize machine state");
		return 1;
	}

	machine_state->is_running = true;
	run_machine(machine_state);
	free_machine(machine_state);
	destroy_platform_context(&sdl_context);
	return 0;
}





