#include "EmulateMWSpaceInvaders.h"

MWState* init_mw_state(Cpu8080* cpu) {
	MWState* mwState = (MWState*)calloc(1, sizeof(MWState));
	if (mwState == NULL) {
		return NULL;
	}
	mwState->ports.input_ports[0] = 0;
	mwState->ports.input_ports[1] = 0;
	mwState->ports.input_ports[2] = 0;
	mwState->ports.shift_value = 0;
	mwState->ports.shift_offset = 0;
	mwState->ports.sound_bits_1 = 0;
	mwState->ports.sound_bits_2 = 0;
	mwState->ports.watchdog = 0;

	InTask in_task = { machine_in, &mwState->ports };
	OutTask out_task = { machine_out, &mwState->ports };
	set_in_out_ports(cpu, in_task, out_task);
	return mwState;
}

void free_MWState(MWState* mwState)
{
	free(mwState);
}

uint8_t* get_frame_buffer(Cpu8080* cpu)
{
	return cpu->state->memory + 0x2400; // Frame buffer start at 0x2400 offset
}

void machine_out(OUTPUT_PORT port, uint8_t value, PortsState* ports_state) {
	switch (port) {
		case SHIFT_OFFSET: {
			ports_state->shift_offset = value & 0b111; // The shift offset is 3 bits wide, so we mask it to 3 bits.
			break;
		}
		case SOUND_1: {
			ports_state->sound_bits_1 = value;
			break;
		}
		case SHIFT_VALUE: {
			ports_state->shift_value = ((uint16_t)value << 8) | (ports_state->shift_value >> 8);
			break;
		}
		case SOUND_2: {
			ports_state->sound_bits_2 = value;
			break;
		}
		case WATCH_DOG: {
			break;
		}
		default: {
			  break; // TODO: Crash in this case
		}
	}
}

uint8_t machine_in(INPUT_PORT port, PortsState* portsState) {
	switch (port) {
		case INPUT_0:
		case INPUT_1:
		case INPUT_2:
			return portsState->input_ports[port];
		case SHIFT_READ: {
			return portsState->shift_value >> (8 - portsState->shift_offset);
		}
		default: {
			return 0; // TODO: Crash in this case
		}
	}
}

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
