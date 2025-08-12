
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


#include "SpaceInvaderMachine.h"
#include "MWDisplay.h"


#include "SDL_platform.h"
#include "SDL_display.h"




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
	return 6;
}

void poll_keys(KeyPress key_presses_buffer[MAX_KEY_PRESSES], void* machine_state) {
	SDL_Event event;
	KEY key;
	KEY_PRESS_TYPE key_press_type;
	int index = 0;
	while (index < MAX_KEY_PRESSES - 1 && SDL_PollEvent(&event))  {
		switch (event.type) {
		case SDL_EVENT_QUIT:
			exit_machine((MachineState*)machine_state);
			return;
		case SDL_EVENT_KEY_DOWN:
			key = key_to_gameKey(event.key);
			key_press_type = KEY_DOWN;
			printf("%d is down\n", key);
			break;
		case SDL_EVENT_KEY_UP:
			key = key_to_gameKey(event.key);
			key_press_type = KEY_UP;
			printf("%d is up\n", key);
			break;
		default:
			continue;
		}
		key_presses_buffer[index] = (KeyPress){ key , PLAYER_1, key_press_type }; // TODO: Need to determine the player.
		index++;
	}
}

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

void sdl_play_sound_effects(bool sound_effects[NUMBER_OF_SOUND_EFFECTS], SDL_CONTEXT* platform_context) {
	for (SOUND_EFFECT_INVADERS sound_effect_invader = 0; sound_effect_invader < NUMBER_OF_SOUND_EFFECTS; sound_effect_invader++) {
		SoundEffectParams_SDL sound_effect_params = platform_context->sound_effects.params[sound_effect_invader];

		if (sound_effects[sound_effect_invader] & !is_playing_sound_effect(sound_effect_params)) {
			play_sound_effect(sound_effect_params);
		}
		
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

int main(int argc, char** argv) {
	if (SDL_Init(SDL_INIT_GAMEPAD) < 0) {
		SDL_Log("SDL_Init errors: %s", SDL_GetError());
		return NULL;
	}
	GAME_ROM game_rom;
	FILE* code_fp;
	int err = fopen_s(&code_fp, argv[1], "rb");
	if (err != NULL) {
		printf("Not able to open the file.");
		exit(1); // What to return here?
	}

	fseek(code_fp, 0L, SEEK_END);
	size_t fileSize = ftell(code_fp);
	fseek(code_fp, 0L, SEEK_SET);

	game_rom.code_buffer = (uint8_t*)malloc(fileSize);
	if (game_rom.code_buffer == NULL) {
		printf("Memory allocation failed.");
		exit(1);
	}
	game_rom.size = fread(game_rom.code_buffer, 1, fileSize, code_fp); // TODO: Validate that return value equal fileSize
	fclose(code_fp);

	char* path = "C:\\Users\\yairy\\Downloads\\invaders\\";
	const char sound_effect_paths[NUMBER_OF_SOUND_EFFECTS][0x100];

	for (int i = 0; i < 9; i++) {
		sprintf_s(sound_effect_paths[i], sizeof(sound_effect_paths[i]), "%s%d.wav", path, i);
	}
	SDL_CONTEXT sdl_context;
	init_sound_effects(&sdl_context.sound_effects, sound_effect_paths);
	init_renderer_SDL(&sdl_context.display);

	DisplayFunctions display_functions = (DisplayFunctions) { render_frame_SDL, destroy_renderer_SDL};
	InputFunctions input_functions = (InputFunctions) { poll_keys };
	TimeFunctions time_functions = (TimeFunctions) { SDL_Delay, get_microseconds_since_start };
	MachineState* machine_state = init_machine(game_rom, 0x10000, &sdl_context, sdl_play_sound_effects, input_functions, display_functions, time_functions); // 16K

	machine_state->is_running = true;
	run_machine(machine_state);
	free_machine(machine_state);
	return 0;
}





