#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


#include "SpaceInvaderMachine.h"
#include "MWDisplay.h"

const int kScreenWidth =  448;
const int kScreenHeight = 512;

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
int tstInterrupt = 1;

KEY key_to_gameKey(SDL_KeyboardEvent keyboardEvent) {
	switch (keyboardEvent.key) {
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
//int input_handling_function(void* data) {
//	MachineState* machineState = (MachineState*)data;
//	SDL_Event event;
//	while (true) {
//		SDL_PollEvent(&event);
//		switch (event.type) {
//			/* Keyboard event */
//			/* Pass the event data onto PrintKeyInfo() */
//		case SDL_EVENT_KEY_DOWN:
//			//machine_key_down()
//			printf("%s is down", SDL_GetKeyName(event.key->keysym));
//		case SDL_EVENT_KEY_UP:
//			printf("%s is up", event.key);
//			//PrintKeyInfo(&event.key);
//			break;
//
//			/* SDL_QUIT event (window close) */
//		case SDL_EVENT_QUIT:
//			break;
//		}
//		SDL_Delay(1);
//	}
//	return 0;
//}

int main(int argc, char** argv) {
	FILE* codeFp;
	int err = fopen_s(&codeFp, argv[1], "rb");
	if (err != NULL) {
		printf("Not able to open the file.");
		exit(1); // What to return here?
	}

	fseek(codeFp, 0L, SEEK_END);
	size_t fileSize = ftell(codeFp);
	fseek(codeFp, 0L, SEEK_SET);

	unsigned char* buffer = (unsigned char*)malloc(fileSize);
	if (buffer == NULL) {
		printf("Memory allocation failed.");
		exit(1);
	}
	size_t size = fread(buffer, 1, fileSize, codeFp); // TODO: Validate that return value equal fileSize
	fclose(codeFp);



	MachineState* machineState = init_machine(size, buffer, 0x10000); // 16K
	

	bool didInit = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD);
	if (didInit == false) {
		SDL_Log("SDL_Init errors: %s", SDL_GetError());
		return -1;
	}

	DisplayParams* displayParams = init_display_params(kScreenWidth, kScreenHeight, (SDL_Color) { 255, 0, 0, 255 });

	uint64_t lastRunTime = get_microseconds_since_start();
	uint64_t nextInterrupt = lastRunTime + 16666;

	SDL_Event event;
	KEY key;
	while (true) {
		uint64_t currentRunTime = get_microseconds_since_start();
		SDL_PollEvent(&event);
		switch (event.type) {
			/* Keyboard event */
			/* Pass the event data onto PrintKeyInfo() */
		case SDL_EVENT_KEY_DOWN:
			key = key_to_gameKey(event.key);
			//machine_key_down()
			printf("%d is down\n", key);
			machine_key_down(key, PLAYER1, &machineState->mwState->ports);
			break;
		case SDL_EVENT_KEY_UP:
			key = key_to_gameKey(event.key);
			machine_key_up(key, PLAYER1, &machineState->mwState->ports);
			printf("%d is up\n", key);
			//PrintKeyInfo(&event.key);
			break;
		}
			/* SDL_QUIT event (window close) */

		if (currentRunTime > nextInterrupt && machineState->cpu->state->interrupt_enable) {
			//generate_interrupt(machineState->cpu->state, 2);
			//generate_interrupt(machineState->cpu->state, 1);


			generate_interrupt(machineState->cpu->state, tstInterrupt + 1);
			//run_CPU(machineState, lastRunTime, currentRunTime - (currentRunTime - lastRunTime) / 2);

			tstInterrupt = (tstInterrupt + 1) % 2;
			//generate_interrupt(machineState->cpu->state, tstInterrupt + 1);
			/*while (machineState->cpu->state->interrupt_enable == false) {
				run_CPU(machineState, 0, 10);
			}*/

			
			//while (machineState->cpu->state->interrupt_enable == false) {
				//run_CPU(machineState, 0, 10);
			//}
			//tstInterrupt = (tstInterrupt + 1) % 2;
			//run_CPU(machineState, lastRunTime, currentRunTime - (currentRunTime - lastRunTime) / 2);

			//generate_interrupt(machineState->cpu->state, 2);



			nextInterrupt = currentRunTime + 8000;
			//nextInterrupt = currentRunTime + 16666;
			if (tstInterrupt == 1) {
				clearRenderer(displayParams);
				fillScreen(displayParams, get_frame_buffer(machineState->cpu));
				reRenderScreen(displayParams);
			}


		}
		
		run_CPU(machineState, lastRunTime, currentRunTime);


		lastRunTime = currentRunTime;
		SDL_Delay(1);
	}

	free_display_params(displayParams);
	free_machine(machineState);
	return 0;
}





