
#include "Emulate8080.h"
#include "Opcodes8080.h"

bool is_pair(SPECIAL_REGISTER special_register) {
	return special_register < NUMBER_OF_REGISTER_PAIRS;
}

Cpu8080* init_cpu_state(size_t buffer_size, uint8_t* code_buffer, size_t memory_size) {
	Cpu8080* cpu = (Cpu8080*)calloc(1, sizeof(Cpu8080));
	if (cpu == NULL) {
		printf("Failed to allocate memory for the CPU");
		return NULL;
	}

	cpu->state = (State8080*)calloc(1, sizeof(State8080));
	if (cpu->state == NULL) {
		free(cpu);
		printf("Failed to allocate memory for the State8080");
		return NULL;
	}

	cpu->state->memory = (uint8_t*)calloc(memory_size, sizeof(uint8_t));
	if (cpu->state->memory == NULL) {
		printf("Failed to allocate memory for the RAM");
		free(cpu->state);
		free(cpu);
		return NULL;
	}
	memcpy_s(cpu->state->memory, memory_size, code_buffer, buffer_size);	
	return cpu;
}
uint16_t get_register_pair(State8080* state, SPECIAL_REGISTER register_pair) {
	switch (register_pair)
	{
	case BC:
		return (state->general_register[B] << 8) | state->general_register[C];
	case DE:
		return (state->general_register[D] << 8) | state->general_register[E];
	case HL:
		return (state->general_register[H] << 8) | state->general_register[L];
	default:
		printf("Error: Invalid register pair %d\n", register_pair);
		return 0; // TODO: throw here
	}
}

void set_register_pair(State8080* state, SPECIAL_REGISTER register_pair, uint16_t value) {
	uint8_t low = value & 0xff;
	uint8_t high = (value >> 8) & 0xff;
	switch (register_pair)
	{
	case BC:
		state->general_register[B] = high;
		state->general_register[C] = low;
		break;
	case DE:
		state->general_register[D] = high;
		state->general_register[E] = low;
		break;
	case HL:
		state->general_register[H] = high;
		state->general_register[L] = low;
		break;
	}
}

uint16_t bytes_to_word(uint16_t low, uint16_t high) {
	return (high << 8) | low;
}

void write_to_memory(State8080* state, uint16_t offset, uint8_t value) {
	/*if (offset >= 0x2400 && offset <= 0x3FFF)
		printf("Write to video memory at %04x: %02x\n", offset, value);*/

	if (offset < 0x2000) {
		printf("Error: Writing to ROM at %04x\n", offset);
		//exit(EXIT_FAILURE);
		//return;
	}
	else if (offset >= 0x4000) {
		printf("Error: Writing outside of RAM at %04x\n", offset);
		//exit(EXIT_FAILURE);
		//return;
	}

	state->memory[offset] = value;
}

uint8_t read_from_memory(State8080* state, uint16_t offset) {

	return state->memory[offset];
}

uint8_t pop_byte(State8080* state) {
	uint8_t result = read_from_memory(state, state->sp);
	state->sp++;
	return result;
}

void push(State8080* state, uint8_t low, uint8_t high) {
	state->sp -= 2;
	write_to_memory(state, state->sp, low);
	write_to_memory(state, state->sp + 1, high);
}

bool parity(int value, int size) {
	value = (value & ((1 << size) - 1));
	int activeBitCount = 0;
	for (int i = 0; i < size; i++) {
		activeBitCount += value & 1;
		value >>= 1;
	}
	return (activeBitCount % 2) == 0;
}

/// <summary>
/// Parse given byte flags into the state.
/// The data book describe the mapping from a byte to each flag.
/// </summary>
/// <param name="state"></param>
/// <param name="flags"></param>
void byte_to_flags(State8080* state, uint8_t flags) {
	state->cc[CARRY] = (flags >> CARRY) & 1;
	state->cc[PARITY] = (flags >> PARITY) & 1;
	state->cc[AC] = (flags >> AC) & 1;
	state->cc[ZERO] = (flags >> ZERO) & 1;
	state->cc[SIGN] = (flags >> SIGN) & 1;
}

uint8_t flags_to_byte(State8080* state) {
	uint8_t flags = (state->cc[CARRY] << CARRY) | (1 << 1) | (state->cc[PARITY] << PARITY) | (state->cc[AC] << AC) | (state->cc[ZERO] << ZERO) | (state->cc[SIGN] << SIGN); // Bit 1 was set to 1 according to the CPU manual
	return flags;
}



void set_flagsZSP(State8080* state, uint8_t value) {
	state->cc[ZERO] = value == 0;
	state->cc[SIGN] = (value & 0x80) != 0;;
	state->cc[PARITY] = parity(value, 8);
}

void set_flags(State8080* state, uint16_t value) {
	state->cc[CARRY] = value > 0xff;
	set_flagsZSP(state, value & 0xff);
}

void op_rst(State8080* state, uint8_t N) {
	uint8_t high = ((state->pc >> 8) & 0xff);
	uint8_t low = state->pc & 0xff;
	push(state, low, high);
	state->pc = N * 8;
}

static void print_state(State8080* state) {
	/*printf("\n\n\n");
	disassemble_8080_op(state->memory, state->pc);
	printf("C=%d,P=%d,S=%d,Z=%d\n", state->cc[CY], state->cc[P],
		state->cc[S], state->cc[Z]);
	printf("\tAF: $%02x%02x BC: $%02x%02x DE: $%02x%02x HL $%02x%02x PC %04x SP %04x\n\n\n",
		state->general_register[A], flags_to_byte(state), state->general_register[B], state->c, state->general_register[D],
		state->general_register[E], state->general_register[H], state->general_register[L], state->pc, state->sp);*/
}

static void unimplemented_instruction(State8080* state)
{
	print_state(state);
	printf("Error: Unimplemented instruction 0x%02x\n", read_from_memory(state, state->pc)); // TODO: add information from the state.
	exit(EXIT_FAILURE);
}

void set_in_out_ports(Cpu8080* cpu, InTask in_task, OutTask out_task)
{
	cpu->in_task = in_task;
	cpu->out_task = out_task;
}

void free_cpu(Cpu8080* cpu) {
	free(cpu->state->memory);
	free(cpu->state);
	free(cpu);
}

static const uint8_t OPCODE_TO_CYCLES[] = {
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x00..0x0f
	4, 10, 7, 5, 5, 5, 7, 4, 4, 10, 7, 5, 5, 5, 7, 4, //0x10..0x1f
	4, 10, 16, 5, 5, 5, 7, 4, 4, 10, 16, 5, 5, 5, 7, 4, //etc
	4, 10, 13, 5, 10, 10, 10, 4, 4, 10, 13, 5, 5, 5, 7, 4,

	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5, //0x40..0x4f
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	5, 5, 5, 5, 5, 5, 7, 5, 5, 5, 5, 5, 5, 5, 7, 5,
	7, 7, 7, 7, 7, 7, 7, 7, 5, 5, 5, 5, 5, 5, 7, 5,

	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4, //0x80..8x4f
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,
	4, 4, 4, 4, 4, 4, 7, 4, 4, 4, 4, 4, 4, 4, 7, 4,

	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11, //0xc0..0xcf
	11, 10, 10, 10, 17, 11, 7, 11, 11, 10, 10, 10, 10, 17, 7, 11,
	11, 10, 10, 18, 17, 11, 7, 11, 11, 5, 10, 5, 17, 17, 7, 11,
	11, 10, 10, 4, 17, 11, 7, 11, 11, 5, 10, 4, 17, 17, 7, 11,
};

int opcode_to_cycles(uint8_t opcode) {
	if (opcode == 0xd3 || opcode == 0xdb) return 3; // IN and OUT operations
	return OPCODE_TO_CYCLES[opcode];
}

uint8_t get_next_opcode(State8080* state) {
	return read_from_memory(state, state->pc);
}

void generate_interrupt(State8080* state, int interrupt_num) {
	op_rst(state, interrupt_num);
	state->interrupt_enable = false;
}

void push_pair(State8080* state, SPECIAL_REGISTER register_pair) {
	uint16_t pairValue = get_register_pair(state, register_pair);
	uint8_t low = (uint8_t)(pairValue & 0xff);
	uint8_t high = (uint8_t)((pairValue >> 8) & 0xff);
	push(state, low, high);
}

void pop_to_pair(State8080* state, SPECIAL_REGISTER register_pair) {
	uint8_t low = pop_byte(state);
	uint8_t high = pop_byte(state);
	uint16_t pair_value = bytes_to_word(low, high);
	set_register_pair(state, register_pair, pair_value);
}

uint8_t read_from_register_pair_address(State8080* state, SPECIAL_REGISTER register_pair) {
	uint16_t address = get_register_pair(state, register_pair);
	return read_from_memory(state, address);
}

void write_to_register_pair_address(State8080* state, SPECIAL_REGISTER register_pair, uint8_t value) {
	uint16_t address = get_register_pair(state, register_pair);
	write_to_memory(state, address, value);
}

int emulate_8080_op(Cpu8080* cpu) {
	State8080* state = cpu->state;
	uint8_t* opcode = &state->memory[state->pc];
	state->pc += 1;
	for (int handler_index = 0; handler_index < handlers_size; handler_index++)
	{
		if (handlers[handler_index].should_handle_func(*opcode)) {
			handlers[handler_index].handle_func(cpu, state, opcode);
			return opcode_to_cycles(*opcode);
		}
	}
	state->pc--;
	unimplemented_instruction(state);

	return 0;
} 

void run_CPU(Cpu8080* cpu, uint64_t real_time_to_run, uint64_t speed_scaling) {
	uint64_t cycles_to_run = real_time_to_run * 2 * speed_scaling; // 2Mhz
	for (int cycles_ran = 0; cycles_ran < cycles_to_run;)
	{
		uint8_t opcode = get_next_opcode(cpu->state);
		emulate_8080_op(cpu);
		cycles_ran += opcode_to_cycles(opcode);
	}
}