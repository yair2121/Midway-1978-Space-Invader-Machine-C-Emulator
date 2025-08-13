#include "MWInput.h"
#include "SpaceInvaderMachine.h"

static INPUT_PORT keypress_to_port(KeyPress key_press) {
	if (key_press.player == PLAYER_1 || key_press.key == START) // START is in port 1 for both players
	{
		return INPUT_1;
	}
	else {
		return INPUT_2;
	}
}

/// <summary>
/// </summary>
/// <param name="key_press"></param>
/// <returns>Bit index in the input port</returns>
static uint8_t keypress_to_bit_index(KeyPress key_press) {
	uint8_t bit_index = key_press.key;
	// For START, Player1: bit 2, Player2: bit 1 (Same as Player2 enum).
	if (key_press.key == START && key_press.player == PLAYER_1) bit_index = 2;
	return bit_index;
}

void machine_key_press(KeyPress key_press, PortsState* portsState) {
	uint8_t port = keypress_to_port(key_press);
	uint8_t bit = 1 << keypress_to_bit_index(key_press);

	if (key_press.type == KEY_DOWN) {
		portsState->input_ports[port] |= bit;
	}
	else {
		portsState->input_ports[port] &= ~bit;
	}
}