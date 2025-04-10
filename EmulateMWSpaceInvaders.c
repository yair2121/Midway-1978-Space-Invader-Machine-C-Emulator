#include "EmulateMWSpaceInvaders.h"


void machine_OUT(uint8_t port, uint8_t value, Ports* portsState) {

	switch (port)
	{
	case 2: {
		portsState->shiftOffset = value & 0b111;
		break;
	}
	case 3: {
		portsState->oPorts3 = value;
		break;
	}
	case 4: {
		portsState->shiftValue = ((uint16_t)value << 8) | (portsState->shiftValue >> 8);
		break;
	}
	case 5: {
		portsState->oPorts5 = value;
		break;
	}
		// TODO: Crash in this case
	}
}


uint8_t machineIN(uint8_t port, Ports* portsState) {
	switch (port)
	{
	case 0:
	case 1:
	case 2:
		return portsState->iPorts[port];
		break;
	case 3: {
		uint16_t shiftedValue = portsState->shiftValue << portsState->shiftOffset;
		shiftedValue = (shiftedValue >> 8) & 0xff;
		return (uint8_t) shiftedValue;
	}
	default: {
		return 0; // TODO: Crash in this case
	}

	}
}

static uint8_t keyToPort(KEY key, PLAYER player) {
	if (player == PLAYER1 || key == START)
	{
		return 1;
	}
	else {
		return 2;
	}
}

static uint8_t keyToBit(KEY key, PLAYER player) {
	if (key == START) {
		return player == PLAYER2 ? 0x2 : 0x4;
	}
	if (key == TILT) return 0x4;
	if (key == SHOOT) return 0x10;
	if (key == LEFT) return 0x20;
	return 0x40; // RIGHT
}

void machineKeyDown(KEY key, PLAYER player, Ports* portsState)
{
	uint8_t port = keyToPort(key, player);
	uint8_t bit = keyToBit(key, player);
	portsState->iPorts[port] |= bit;
}

void machineKeyUP(KEY key, PLAYER player, Ports* portsState)
{
	uint8_t port = keyToPort(key, player);
	uint8_t bit = ~keyToBit(key, player);
	portsState->iPorts[port] &= bit;
}
