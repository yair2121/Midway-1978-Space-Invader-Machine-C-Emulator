#pragma once
#include <inttypes.h>

#include "Emulate8080.h"

typedef enum KEY {
	LEFT = 0,
	RIGHT = 1,
	SHOOT = 2,
	START = 3,
	TILT = 4,
	KEY_COUNT = 5
} KEY;

typedef enum PLAYER {
	PLAYER1 = 0,
	PLAYER2 = 1,
	PLAYER_COUNT = 2,
	IRRELEVANT = 3
} PLAYER;


typedef struct Ports {
	uint8_t iPorts[3];
	uint16_t shiftValue; // Input Port 3.
	uint8_t shiftOffset; // Output port 2.
	uint8_t oPorts3;
	uint8_t oPorts5;
	uint8_t oPorts6; // Watchdog port, need to check what it is
} Ports;

typedef struct MWState {
	Ports ports;
} MWState;



MWState* init_mw_state(Cpu8080* cpu);

void machine_OUT(uint8_t port, uint8_t value, Ports* portsState);

uint8_t machine_IN(uint8_t port, Ports* portsState);

void machine_key_down(KEY key, PLAYER player, Ports* portsState);
void machine_key_up(KEY key, PLAYER player, Ports* portsState);