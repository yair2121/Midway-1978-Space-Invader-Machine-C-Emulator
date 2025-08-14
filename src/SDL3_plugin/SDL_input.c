#include "SDL_input.h"

/// <summary>
/// Currently only supporting Player1.
/// </summary>
static const KeyMapping key_mappings[] = {
	{SDLK_LEFT , {LEFT, PLAYER_1}},
	{SDLK_RIGHT , {RIGHT, PLAYER_1}},
	{SDLK_SPACE , {SHOOT, PLAYER_1}},
	{SDLK_RETURN , {START, PLAYER_1}},
	{SDLK_RSHIFT , {COIN, IRRELEVANT}},
};

static KeyPress key_event_to_game_key(SDL_Event key_event) {
	for (size_t i = 0; i < SDL_arraysize(key_mappings); ++i) {
		if (key_mappings[i].sdl_key == key_event.key.key) {
			KeyPress key_press = key_mappings[i].key_press;
			key_press.type = key_event.type == SDL_EVENT_KEY_DOWN ? KEY_DOWN : KEY_UP;
			return key_press;
		}
	}
	return INVALID_KEY_PRESS; // Not mapped
}

void poll_keys(KeyPress key_presses_buffer[MAX_KEY_PRESSES], MachineState* machine_state) {
	SDL_PumpEvents();
	SDL_Event events[MAX_KEY_PRESSES];
	int count = SDL_PeepEvents(events, MAX_KEY_PRESSES, SDL_GETEVENT, SDL_EVENT_KEY_DOWN, SDL_EVENT_KEY_UP);

	int index = 0;
	for (int i = 0; i < count && index < MAX_KEY_PRESSES; i++) {
		KeyPress key_press = key_event_to_game_key(events[i]);
		if (is_valid_key_press(key_press) == false) continue;

		//key_press.type = events[i].type == SDL_EVENT_KEY_DOWN ? KEY_DOWN : KEY_UP;
		SDL_Log("Key %d is %s", key_press.key, key_press.type == KEY_DOWN ? "down" : "up");
		key_presses_buffer[index++] = key_press;
	}
}

bool init_input_sdl() {
	return SDL_Init(SDL_INIT_GAMEPAD);
}

void destroy_input_sdl() {
	SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}