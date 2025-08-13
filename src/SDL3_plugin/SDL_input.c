#include "SDL_input.h"

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

