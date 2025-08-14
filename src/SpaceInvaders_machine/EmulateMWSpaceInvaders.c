#include "EmulateMWSpaceInvaders.h"

MWState* init_mw_state(Cpu8080* cpu) {
	MWState* mwState = (MWState*)calloc(1, sizeof(MWState));
	if (mwState == NULL) {
		printf("Failed to allocate memory for the MWState");
		return NULL;
	}

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
	return cpu->state->memory + 0x2400; 
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

uint8_t machine_in(INPUT_PORT port, PortsState* ports_state) {
	switch (port) {
		case INPUT_0:
		case INPUT_1:
		case INPUT_2:
			return ports_state->input_ports[port];
		case SHIFT_READ: {
			return ports_state->shift_value >> (8 - ports_state->shift_offset);
		}
		default: {
			return 0; // TODO: Crash in this case
		}
	}
}
