
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include "SDL_platform.h"

static uint64_t get_microseconds_since_start() {
	static uint64_t start_counter = 0;
	static uint64_t frequency = 0;

	if (start_counter == 0) {
		start_counter = SDL_GetPerformanceCounter();
		frequency = SDL_GetPerformanceFrequency();
	}

	uint64_t current_counter = SDL_GetPerformanceCounter();
	uint64_t elapsed = current_counter - start_counter;
	return (elapsed * 1000000) / frequency;
}


static KEY key_to_gameKey(SDL_KeyboardEvent keyboard_event) {
	switch (keyboard_event.key) {
	case SDLK_LEFT:
		return LEFT;
	case SDLK_RIGHT:
		return RIGHT;
	case SDLK_D:
		return SHOOT;
	case SDLK_E:
		return START;
	case SDLK_R:
		return COIN;
	}
	return KEY_COUNT;
}

void poll_keys(KeyPress key_presses_buffer[MAX_KEY_PRESSES], MachineState* machine_state) {
	SDL_PumpEvents();
	SDL_Event events[MAX_KEY_PRESSES];
	int count = SDL_PeepEvents(events, MAX_KEY_PRESSES, SDL_GETEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);

	int index = 0;
	for (int i = 0; i < count && index < MAX_KEY_PRESSES; i++) {
		KEY key = key_to_gameKey(events[i].key);
		KEY_PRESS_TYPE type = events[i].type == SDL_EVENT_KEY_DOWN ? KEY_DOWN : KEY_UP;
		printf("%d is %s\n", key, type == KEY_DOWN ? "down" : "up");
		key_presses_buffer[index++] = (KeyPress){ key, PLAYER_1, type };
	}
}

void sdl_handle_system_events(MachineState* machine_state) {
	SDL_Event event;
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_WINDOW_CLOSE_REQUESTED, SDL_EVENT_WINDOW_CLOSE_REQUESTED) > 0) {
		exit_machine(machine_state);
	}
}

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





