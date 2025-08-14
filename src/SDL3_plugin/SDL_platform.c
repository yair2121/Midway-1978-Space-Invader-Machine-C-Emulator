#include "SDL_platform.h"


void sdl_play_sound_effects(bool sound_effects[NUMBER_OF_SOUND_EFFECTS], SDL_CONTEXT* platform_context) {
	for (SOUND_EFFECT_INVADERS sound_effect_invader = 0; sound_effect_invader < NUMBER_OF_SOUND_EFFECTS; sound_effect_invader++) {
		SoundEffectParams_SDL sound_effect_params = platform_context->sound_effects.params[sound_effect_invader];

		if (sound_effects[sound_effect_invader] & !is_playing_sound_effect(sound_effect_params)) {
			play_sound_effect(sound_effect_params);
		}

	}
}

uint64_t get_microseconds_since_start() {
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

void sdl_handle_system_events(MachineState* machine_state) {
	SDL_PumpEvents();
	SDL_Event event;
	if (SDL_PeepEvents(&event, 1, SDL_GETEVENT, SDL_EVENT_WINDOW_CLOSE_REQUESTED, SDL_EVENT_WINDOW_CLOSE_REQUESTED) > 0) {
		exit_machine(machine_state);
	}
}

void destroy_platform_context(SDL_CONTEXT* platform_context) {
	SDL_Log("Destroying SDL platform context\n");
	if (platform_context != NULL) {
		destroy_renderer_sdl(&platform_context->display);
		destroy_input_sdl();
		destroy_sound_effects_sdl(&platform_context->sound_effects);
	}
	SDL_Quit();
}