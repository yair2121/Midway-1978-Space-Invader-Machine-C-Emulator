#include "SDL_sound.h"

#include "SDL_platform.h"

bool is_playing_sound_effect(SoundEffectParams_SDL sound_effect_params) {
	return SDL_GetAudioStreamAvailable(sound_effect_params.stream) > 0;
}

void play_sound_effect(SoundEffectParams_SDL sound_effect_params) {
	if (SDL_PutAudioStreamData(sound_effect_params.stream,
		sound_effect_params.audio_buffer,
		sound_effect_params.audio_length) < 0) {
		printf("Failed to queue audio data for sound %d: %s\n", 0, SDL_GetError());
	}
}


void init_sound_effects(SoundEffects_SDL* sound_effects_sdl, const char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100]) {
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		SDL_Log("SDL_Init errors: %s", SDL_GetError());
		return NULL;
	}

	SDL_AudioDeviceID device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (device == 0) {
		printf("Failed to open audio device: %s\n", SDL_GetError());
		SDL_Quit();
		return -1;
	}
	sound_effects_sdl->device = device;

	for (SOUND_EFFECT_INVADERS i = 0; i < NUMBER_OF_SOUND_EFFECTS; i++) {
		int port_sound_effects_index = i;
		if (i >= NUMBER_OF_SOUND_EFFECTS_PORT_3) {
			port_sound_effects_index -= NUMBER_OF_SOUND_EFFECTS_PORT_3; // Adjust index for port 5 effects
		}
		else {
			port_sound_effects_index = i; // Use the same index for port 3 effects
		}
		SoundEffectParams_SDL* sound_effect_params = &sound_effects_sdl->params[i];
		if (!SDL_LoadWAV(sound_effect_paths[i], &sound_effect_params->spec, &sound_effect_params->audio_buffer, &sound_effect_params->audio_length)) {
			printf("Failed to load WAV: %s\n", SDL_GetError());
			SDL_Quit();
			exit(-1);
		}

		sound_effect_params->stream = SDL_CreateAudioStream(&sound_effect_params->spec, NULL);
		if (!sound_effect_params->stream) {
			printf("Failed to create audio stream for sound %d: %s\n", i, SDL_GetError());
			return -1;
		}

		if (SDL_BindAudioStream(device, sound_effect_params->stream) < 0) {
			printf("Failed to bind audio stream for sound %d: %s\n", i, SDL_GetError());
			return -1;
		}
	}
}