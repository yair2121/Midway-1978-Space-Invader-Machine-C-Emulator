#include "EmulateMWSpaceInvaders.h"

MWState* init_mw_state(Cpu8080* cpu) {
	MWState* mwState = (MWState*)calloc(1, sizeof(MWState));
	if (mwState == NULL) {
		return NULL;
	}
	mwState->ports.iPorts[0] = 0;
	mwState->ports.iPorts[1] = 0;
	mwState->ports.iPorts[2] = 0;
	mwState->ports.shiftValue = 0;
	mwState->ports.shiftOffset = 0;
	mwState->ports.oPorts3 = 0;
	mwState->ports.oPorts5 = 0;
	mwState->ports.oPorts6 = 0;

	InTask inTask = { machine_IN, &mwState->ports };
	OutTask outTask = { machine_OUT, &mwState->ports };
	//machineState->mwState->ports.shiftValue = 5;
	set_in_out_ports(cpu, inTask, outTask);
	return mwState;
}

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

uint8_t machine_IN(uint8_t port, Ports* portsState) {
	switch (port)
	{
	case 0:
		return 1;
	case 1:
		return portsState->iPorts[port];
	case 2:
		return 0;
	case 3: {
		uint16_t shiftedValueResult = portsState->shiftValue << portsState->shiftOffset;
		shiftedValueResult = (shiftedValueResult >> 8) & 0xff;
		return (uint8_t) shiftedValueResult;
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
	if (key == COIN) return 0x1;
	if (key == TILT) return 0x4;
	if (key == SHOOT) return 0x10;
	if (key == LEFT) return 0x20;

	return 0x40; // RIGHT
}

void machine_key_down(KEY key, PLAYER player, Ports* portsState)
{
	uint8_t port = keyToPort(key, player);
	uint8_t bit = keyToBit(key, player);
	portsState->iPorts[port] |= bit;
}

void machine_key_up(KEY key, PLAYER player, Ports* portsState)
{
	uint8_t port = keyToPort(key, player);
	uint8_t bit = ~keyToBit(key, player);
	portsState->iPorts[port] &= bit;
}
