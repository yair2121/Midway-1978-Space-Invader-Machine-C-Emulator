#include "SDL_sound.h"

#include "SDL_platform.h"

bool is_playing_sound_effect(SoundEffectParams_SDL sound_effect_params) {
	return SDL_GetAudioStreamAvailable(sound_effect_params.stream) > 0;
}

void play_sound_effect(SoundEffectParams_SDL sound_effect_params) {
	if (SDL_PutAudioStreamData(sound_effect_params.stream,
		sound_effect_params.audio_buffer,
		sound_effect_params.audio_length) == false) {
		SDL_Log("Failed to queue audio data for sound %d: %s\n", 0, SDL_GetError());
	}
}


bool init_sound_effects_sdl(SoundEffects_SDL* sound_effects_sdl, const char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100]) {
	if (SDL_Init(SDL_INIT_AUDIO) == false) {
		SDL_Log("SDL_Init errors");
		return false;
	}

	SDL_AudioDeviceID device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
	if (device == 0) {
		SDL_Log("Failed to open audio device\n");
		return false;
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
			SDL_Log("Failed to load WAV\n");
			return false;
		}

		sound_effect_params->stream = SDL_CreateAudioStream(&sound_effect_params->spec, NULL);
		if (!sound_effect_params->stream) {
			SDL_Log("Failed to create audio stream for sound %d\n", i);
			return false;
		}

		if (SDL_BindAudioStream(device, sound_effect_params->stream) < 0) {
			SDL_Log("Failed to bind audio stream for sound %d\n", i);
			return false;
		}
	}
	return true;
}

void destroy_sound_effects_sdl(SoundEffects_SDL* sound_effects_sdl) {
	for (SOUND_EFFECT_INVADERS sound_effect_invaders = UFO; sound_effect_invaders < NUMBER_OF_SOUND_EFFECTS; sound_effect_invaders++) {
		SoundEffectParams_SDL* sound_effect_sdl = &sound_effects_sdl->params[sound_effect_invaders];
		if (sound_effect_sdl->stream) {
			SDL_DestroyAudioStream(sound_effect_sdl->stream);
		}
		if (sound_effect_sdl->audio_buffer) {
			SDL_free(sound_effect_sdl->audio_buffer);
		}
	}

	if (sound_effects_sdl->device != 0) {
		SDL_CloseAudioDevice(sound_effects_sdl->device);
	}
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
}
