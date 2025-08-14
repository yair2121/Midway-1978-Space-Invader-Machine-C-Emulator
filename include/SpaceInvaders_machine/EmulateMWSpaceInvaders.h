#pragma once
#include <inttypes.h>

#include "Emulate8080.h"

typedef enum INPUT_PORT {
	INPUT_0 = 0,
	INPUT_1 = 1,
	INPUT_2 = 2,
	NUMBER_OF_INPUT_PORTS = 3, // Total number of input ports
	SHIFT_READ = 3, // Read from the shift register
} INPUT_PORT;

typedef enum OUTPUT_PORT {
	SHIFT_OFFSET = 2, // Start from 2 to match the Space Invaders machine port layout
	SOUND_1 = 3,
	SHIFT_VALUE = 4,
	SOUND_2 = 5,
	WATCH_DOG = 6,
} OUTPUT_PORT;

typedef struct PortsState {
	uint8_t input_ports[NUMBER_OF_INPUT_PORTS];
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

/// <summary>
/// Frame buffer start at 0x2400 offset
/// </summary>
/// <param name="cpu"></param>
/// <returns></returns>
int8_t* get_frame_buffer(Cpu8080* cpu);

void machine_out(OUTPUT_PORT port, uint8_t value, PortsState* ports_state);

uint8_t machine_in(INPUT_PORT port, PortsState* ports_state);

