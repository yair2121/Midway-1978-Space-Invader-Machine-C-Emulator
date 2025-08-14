#pragma once
#include <stdbool.h>

typedef struct PortsState PortsState;


#define INVALID_KEY_PRESS (KeyPress) { INVALID_KEY, INVALID_PLAYER, INVALID_KEY_TYPE }
#define MAX_KEY_PRESSES 10

typedef enum KEY_PRESS_TYPE {
	INVALID_KEY_TYPE = -1,
	KEY_UP = 0,
	KEY_DOWN,
	KEY_PRES_TYPE_COUNT
} KEY_PRESS_TYPE;

typedef enum KEY {
	INVALID_KEY = -1,
	COIN = 0,
	START = 1,
	TILT = 2, // Not used in Space Invader 
	KEY_PADDING = 3, // Padding to match the original Space Invaders machine layout
	SHOOT = 4,
	LEFT = 5,
	RIGHT = 6,
	KEY_COUNT
} KEY;

typedef enum PLAYER {
	INVALID_PLAYER = -1,
	PLAYER_1 = 1, // Start from 1 to fit the Space Invaders machine port layout.
	PLAYER_2 = 2,
	PLAYER_COUNT = PLAYER_2,
	IRRELEVANT = PLAYER_1,
} PLAYER;

typedef struct KeyPress {
	KEY key;
	PLAYER player;
	KEY_PRESS_TYPE type;
} KeyPress;

void machine_key_press(KeyPress key_press, PortsState* ports_state);
bool is_valid_key_press(KeyPress keyPress);