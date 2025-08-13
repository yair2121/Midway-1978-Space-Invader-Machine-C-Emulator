
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDL_platform.h"



int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_GAMEPAD) < 0) {
		SDL_Log("SDL_Init errors: %s", SDL_GetError());
		return NULL;
	}

	char* game_rom_path = argv[1];
	char* sound_directory = argv[2];

	char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100];
	for (int i = 0; i < NUMBER_OF_SOUND_EFFECTS; i++) {
		sprintf_s(sound_effect_paths[i], sizeof(sound_effect_paths[i]), "%s%d.wav", sound_directory, i);
	}


	SDL_CONTEXT sdl_context;
	init_sound_effects(&sdl_context.sound_effects, sound_effect_paths);
	init_renderer_SDL(&sdl_context.display);

	DisplayFunctions display_functions = (DisplayFunctions) { render_frame_SDL, destroy_renderer_SDL};
	EventsFunctions events_functions = (EventsFunctions) { poll_keys, sdl_handle_system_events };
	TimeFunctions time_functions = (TimeFunctions) { SDL_Delay, get_microseconds_since_start };
	MachineState* machine_state = init_machine(game_rom_path, &sdl_context, sdl_play_sound_effects, events_functions, display_functions, time_functions); // 16K

	machine_state->is_running = true;
	run_machine(machine_state);
	free_machine(machine_state);
	return 0;
}





