#pragma once

#include <SDL3/SDL.h>

#include "SDL_display.h"
#include "SDL_sound.h"
#include "SDL_input.h"

uint64_t get_microseconds_since_start();
typedef struct SDL_CONTEXT {
	DisplayParams_SDL display;
	SoundEffects_SDL sound_effects;
} SDL_CONTEXT;

void sdl_play_sound_effects(bool sound_effects[NUMBER_OF_SOUND_EFFECTS], SDL_CONTEXT* platform_context);

void sdl_handle_system_events(MachineState* machine_state);

void destroy_platform_context(SDL_CONTEXT* platform_context);