#pragma once
#include <inttypes.h>

#include "Emulate8080.h"


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
	TILT = 2,
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
	IRRELEVANT = 3,
} PLAYER;

typedef enum INPUT_PORT {
	INPUT_0 = 0,
	INPUT_1 = 1,
	INPUT_2 = 2,
	SHIFT_READ = 3, // Read from the shift register
} INPUT_PORT;

typedef enum OUTPUT_PORT {
	SHIFT_OFFSET = 2, // Start from 2 to match the Space Invaders machine port layout
	SOUND_1 = 3,
	SHIFT_VALUE = 4,
	SOUND_2 = 5,
	WATCH_DOG = 6,
} OUTPUT_PORT;

typedef struct KeyPress {
	KEY key;
	PLAYER player;
	KEY_PRESS_TYPE type;
} KeyPress;

#define INVALID_KEY_PRESS (KeyPress) { INVALID_KEY, INVALID_PLAYER, INVALID_KEY_TYPE }
#define MAX_KEY_PRESSES 10

typedef struct PortsState {
	uint8_t input_ports[3];
	uint8_t shift_offset; // Output port 2.
	uint8_t sound_bits_1;
	uint16_t shift_value; // Input Port 3.
	uint8_t sound_bits_2;
	uint8_t watchdog;
} PortsState;

typedef struct MWState {
	PortsState ports;
} MWState;

MWState* init_mw_state(Cpu8080* cpu);
void free_MWState(MWState* mwState);
int8_t* get_frame_buffer(Cpu8080* cpu);

void machine_out(OUTPUT_PORT port, uint8_t value, PortsState* ports_state);

uint8_t machine_in(INPUT_PORT port, PortsState* ports_state);

void machine_key_press(KeyPress key_press, PortsState* ports_state);
