#pragma once
#include <inttypes.h>

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

//const uint8_t MAP_TO_BIT[PLAYER_COUNT][KEY_COUNT] = { {0x20, 0x40, 0x10, 0x4,}, {1} };

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




void machineOUT(uint8_t port, uint8_t value, Ports* portsState);

uint8_t machineIN(uint8_t port, Ports* portsState);

void machineKeyDown(KEY key, PLAYER player, Ports* portsState);
void machineKeyUP(KEY key, PLAYER player, Ports* portsState);