#pragma once
#include "SpaceInvaderMachine.h"

typedef struct SDL_CONTEXT SDL_CONTEXT;

typedef struct SoundEffect_SDL {
	SDL_AudioSpec spec; // Audio specification for the sound effect
	uint8_t* audio_buffer;
	uint32_t audio_length;
	SDL_AudioStream* stream;
} SoundEffectParams_SDL;

typedef struct SoundEffects_SDL {
	SoundEffectParams_SDL params[NUMBER_OF_SOUND_EFFECTS];
	SDL_AudioDeviceID device;
} SoundEffects_SDL;


bool is_playing_sound_effect(SoundEffectParams_SDL sound_effect_params);

void play_sound_effect(SoundEffectParams_SDL sound_effect_params);

bool init_sound_effects_sdl(SoundEffects_SDL* sound_effects_sdl, const char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100]);

void destroy_sound_effects_sdl(SoundEffects_SDL* sound_effects_sdl);