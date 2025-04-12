#include "Emulate8080.h"
#include "Disassembler8080.h"
#include <stdio.h>

static inline uint16_t pair_to_address(uint8_t low, uint8_t high) {
	return ((uint16_t)high << 8) | low;
}
static uint8_t pop_byte(State8080* state) {
	uint8_t result = state->memory[state->sp];
	state->sp++;
	return result;
}
static void write_to_memory(State8080* state, uint16_t offset, uint8_t value) {
	if (offset < 0x2000) {
		printf("Error: Writing to ROM at %04x\n", offset);
		//exit(EXIT_FAILURE);
		return;
	}
	else if (offset >= 0x4000) {
		printf("Error: Writing outside of RAM at %04x\n", offset);
		//exit(EXIT_FAILURE);
		return;
	}
	state->memory[offset] = value;
}

static void push(State8080* state, uint8_t low, uint8_t high) {
	state->sp -= 2;
	write_to_memory(state, state->sp, low);
	write_to_memory(state, state->sp + 1, high);
}

static bool parity(int value, int size) {
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
static void byte_to_flags(State8080* state, uint8_t flags) {
	state->cc.cy = flags & 1;
	state->cc.p = (flags >> 2) & 1;
	state->cc.ac = (flags >> 4) & 1;
	state->cc.z = (flags >> 6) & 1;
	state->cc.s = (flags >> 7) & 1;
}

static uint8_t flags_to_byte(State8080* state) {
	uint8_t flags = state->cc.cy | 1 << 1 | state->cc.p << 2 | state->cc.ac << 4 | state->cc.z << 6 | state->cc.s << 7; // Bit 1 was set to 1 according to the CPU manual
	return flags;
}



static void set_flagsZSP(State8080* state, uint16_t value) {
	state->cc.z = (value & 0xff) == 0;
	state->cc.s = (value & 0x80) == 0x80;
	state->cc.p = parity(value, 8);
}

static void set_flags(State8080* state, uint16_t value) {
	state->cc.cy = value > 0xff;
	set_flagsZSP(state, value);
}

static void ret_opcode(State8080* state) {
	uint8_t low = pop_byte(state);
	state->pc = pair_to_address(low, pop_byte(state));
}

static void jmp_opcode(State8080* state, uint8_t low, uint8_t high) {
	state->pc = pair_to_address(low, high);
}

static void call_opcode(State8080* state, uint8_t adr_low, uint8_t adr_high) {
	uint16_t ret_address = state->pc + 2;
	uint8_t high = (uint8_t)((ret_address >> 8) & 0xff);
	uint8_t low = (uint8_t)(ret_address & 0xff);
	push(state, low, high);
	state->pc = pair_to_address(adr_low, adr_high);
}

static void print_state(State8080* state) {
	/*printf("\n\n\n");
	disassemble_8080_op(state->memory, state->pc);
	printf("C=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p,
		state->cc.s, state->cc.z);
	printf("\tAF: $%02x%02x BC: $%02x%02x DE: $%02x%02x HL $%02x%02x PC %04x SP %04x\n\n\n",
		state->a, flags_to_byte(state), state->b, state->c, state->d,
		state->e, state->h, state->l, state->pc, state->sp);*/
}

static void unimplemented_instruction(State8080* state)
{
	print_state(state);
	printf("Error: Unimplemented instruction 0x%02x\n", state->memory[state->pc]); // TODO: add information from the state.
	exit(EXIT_FAILURE);
}

Cpu8080* init_cpu_state(size_t bufferSize, uint8_t* codeBuffer, size_t memorySize) {
	Cpu8080* cpu = (Cpu8080*)calloc(1, sizeof(Cpu8080));
	if (cpu == NULL) {
		return NULL;
	}

	cpu->state = (State8080*)calloc(1, sizeof(State8080));
	if (cpu->state == NULL) {
		free(cpu);
		return NULL;
	}

	cpu->state->memory = (uint8_t*)calloc(memorySize, sizeof(uint8_t));
	if (cpu->state->memory == NULL) {
		free(cpu->state);
		free(cpu);
		return NULL;
	}
	memcpy_s(cpu->state->memory, memorySize, codeBuffer, bufferSize);
	//cpu->state->interrupt_enable = true; // TODO: ????
	return cpu;
}

void set_in_out_ports(Cpu8080* cpu, InTask inTask, OutTask outTask)
{
	cpu->inTask = inTask;
	cpu->outTask = outTask;
}

void free_cpu(Cpu8080* cpu) {
	free(cpu->state->memory);
	free(cpu->state);
	//free(cpu->inTask);
	//free(cpu->outTask);
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
	return state->memory[state->pc];
}

void generate_interrupt(State8080* state, int interrupt_num) {
	//perform "PUSH PC"
	//push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));
	push(state, (state->pc & 0xff), (state->pc & 0xFF00) >> 8);


	//Set the PC to the low memory vector.    
	//This is identical to an "RST interrupt_num" instruction.
	state->pc = 8 * interrupt_num;

	state->interrupt_enable = false;
}

static uint16_t get_register_pair(State8080* state, REGISTER_PAIR register_pair) {
	switch (register_pair)
	{
	case BC:
		return (state->b << 8) | state->c;
	case DE:
		return (state->d << 8) | state->e;
	case HL:
		return (state->h << 8) | state->l;
	default:
		return 0; // TODO: throw here
	}
}

static void push_pair(State8080* state, REGISTER_PAIR register_pair) {
	uint16_t pairValue = get_register_pair(state, register_pair);
	uint8_t low = (uint8_t)(pairValue & 0xff);
	uint8_t high = (uint8_t)((pairValue >> 8) & 0xff);
	push(state, low, high);
}

static uint8_t read_from_register_pair_address(State8080* state, REGISTER_PAIR register_pair) {
	uint16_t address = get_register_pair(state, register_pair);
	return state->memory[address];
}

static void write_to_register_pair_address(State8080* state, REGISTER_PAIR register_pair, uint8_t value) {
	uint16_t address = get_register_pair(state, register_pair);
	write_to_memory(state, address, value);
}

static void set_register_pair(State8080* state, REGISTER_PAIR register_pair, uint16_t value) {
	uint8_t high = (value >> 8) & 0xff;
	uint8_t low = value & 0xff;
	switch (register_pair)
	{
	case BC:
		state->b = high;
		state->c = low;
		break;
	case DE:
		state->d = high;
		state->e = low;
		break;
	case HL:
		state->h = high;
		state->l = low;
		break;
	}
}


int count = 0;
int cycles = 0;
int emulate_8080_op(Cpu8080* cpu) {
	State8080* state = cpu->state;
	uint8_t* opcode = &state->memory[state->pc];
	count++;
	cycles += opcode_to_cycles(opcode);
	if (count >= 0xa431) {
		print_state(cpu->state);
		//printf("Cycles: %d\n", cycles);
		//count = 1;
		cycles = cycles;
	}
	//print_state(state);
	state->pc += 1;

	switch (*opcode)
	{
	case 0x00:
	//case 0x08:
	//case 0x18:
	//case 0x28:
	//case 0x38:
	//case 0xcb:
	//case 0xd9:
	//case 0xdd:
	//case 0xed:
	//case 0xfd:
		break;

	case 0x01: { // LXI B,D16
		state->c = opcode[1];
		state->b = opcode[2];
		state->pc += 2;
		break;
	}
	//case 0x02: {
	//	uint16_t bc = get_register_pair(state, BC);
	//	write_to_memory(state, bc, state->a);
	//	break;
	//}
	case 0x03: { // INX B
		uint16_t bc = get_register_pair(state, BC);
		set_register_pair(state, BC, bc + 1);
		break;
	}
	case 0x04: { // INR B
		state->b++;
		set_flagsZSP(state, state->b);
		break;

	}
	case 0x05: { // DCR B
		state->b--;
		set_flagsZSP(state, state->b);
		break;
	}
	case 0x06: { // MVI B, D8
		state->b = opcode[1];
		state->pc++;
		break;
	}
	case 0x07: { // RLC
		state->cc.cy = state->a >> 7;
		state->a = state->a << 1;
		state->a = (state->a & 0xFE) | state->cc.cy; // Clear the last bit and set it to the carry
		break;
	}
	case 0x09: { // DAD B
		uint32_t hl = get_register_pair(state, HL);
		uint32_t bc = get_register_pair(state, BC);
		uint32_t combined = hl + bc;
		set_register_pair(state, HL, combined & 0xffff);
		state->cc.cy = ((combined & 0xffff0000) > 0);
		break;
	}
	case 0x0a: { // LDAX B
		state->a = read_from_register_pair_address(state, BC);
		break;
	}
	case 0x0d: { // DCR C
		state->c--;
		set_flagsZSP(state, state->c);
		break;
	}
	case 0x0c: { // INR C 
		state->c++;
		set_flagsZSP(state, state->c);
		break;
	}
	case 0x0e: { // MVI C,D8
		state->c = opcode[1];
		state->pc++;
		break;
	}
	case 0x0f: { // RRC
		state->cc.cy = state->a & 1;
		state->a = (state->a >> 1) | (state->cc.cy << 7);
		break;
	}
	case 0x11: { // LXI D,D16
		state->e = opcode[1];
		state->d = opcode[2];
		state->pc += 2;
		break;
	}
	case 0x13: { // INC DE
		uint16_t de = get_register_pair(state, DE);
		set_register_pair(state, DE, de + 1);
		break;
	}
	case 0x14: { // INR D
		set_flagsZSP(state, ++state->d);
		break;
	}
	case 0x15: { // DCR D
		set_flagsZSP(state, --state->d);
		break;
	}
	case 0x16: { // MVI D,D8
		state->d = opcode[1];
		state->pc++;
		break;
	}
	case 0x19: { // DAD H
		uint32_t hl = get_register_pair(state, HL);
		uint32_t de = get_register_pair(state, DE);
		uint32_t combined = hl + de;
		set_register_pair(state, HL, combined & 0xffff);
		state->cc.cy = ((combined & 0xffff0000) != 0);
		break;
	}
	case 0x1a: { // LDAX D
		state->a = read_from_register_pair_address(state, DE);
		break;
	}

	case 0x1f: { // RAR
		state->cc.cy = state->a & 1;
		uint8_t prevBit7 = state->a & 0x80;
		state->a >>= 1;
		state->a = (state->a & 0x7f) | prevBit7;
		break;
	}
	case 0x21: { // LXI H,D16
		state->l = opcode[1];
		state->h = opcode[2];
		state->pc += 2;
		break;
	}
	case 0x22: { // SHLD adr
		uint16_t adr = pair_to_address(opcode[1], opcode[2]);
		write_to_memory(state, adr, state->l);
		write_to_memory(state, adr + 1, state->h);
		state->pc += 2;
		break;
	}
	case 0x23: { // INX H
		uint16_t hl = get_register_pair(state, HL);
		set_register_pair(state, HL, hl + 1);
		break;
	}
	case 0x26: { // MVI H,D8
		state->h = opcode[1];
		state->pc++;
		break;
	}
	case 0x29: { // DAD H
		uint32_t hl = get_register_pair(state, HL);
		hl += hl;
		set_register_pair(state, HL, hl & 0xffff);
		state->cc.cy = ((hl & 0xffff0000) != 0);
		break;
	}
	case 0x2a: { // LHLD adr
		uint16_t address = pair_to_address(opcode[1], opcode[2]);	
		state->l = state->memory[address];
		state->h = state->memory[address + 1];
		state->pc += 2;
		break;
	}
	case 0x2b: { // DCX H
		uint16_t hl = get_register_pair(state, HL);
		set_register_pair(state, HL, hl - 1);
		break;
	}
	case 0x2c: { // INR L
		state->l++;
		set_flagsZSP(state, state->l);
		break;
	}
	case 0x2e: { // MVI L, D8
		state->l = opcode[1];
		state->pc++;
		break;
	}
	case 0x2f: { // CMA
		state->a = ~state->a;
		break;
	}
	case 0x31: { // LXI SP, D16
		state->sp = pair_to_address(opcode[1], opcode[2]);
		state->pc += 2;
		break;
	}
	case 0x32: { // STA adr
		uint16_t address = pair_to_address(opcode[1], opcode[2]);
		write_to_memory(state, address, state->a);
		state->pc += 2;
		break;
	}
	case 0x34: { // INR M
		uint16_t hl = get_register_pair(state, HL);
		write_to_memory(state, hl, state->memory[hl] + 1);
		set_flagsZSP(state, state->memory[hl]);
		break;
	}
	case 0x35: { // DCR M
		uint16_t hl = get_register_pair(state, HL);
		write_to_memory(state, hl, state->memory[hl] - 1);
		set_flagsZSP(state, state->memory[hl]);
		break;
	}
	case 0x36: { // MVI M,D8
		write_to_register_pair_address(state, HL, opcode[1]);
		state->pc++;
		break;
	}
	case 0x37: { // STC
		state->cc.cy = 1;
		break;
	}
	case 0x3a: { // LDA adr
		uint16_t address = pair_to_address(opcode[1], opcode[2]);
		state->a = state->memory[address];
		state->pc += 2;
		break;
	}
	case 0x3c: { // INR A
		state->a++;
		set_flagsZSP(state, state->a);
		break;
	}
	case 0x3d: { // DCR A
		state->a--;
		set_flagsZSP(state, state->a);
		break;
	}
	case 0x3e: { // MVI A,D8
		state->a = opcode[1];
		state->pc++;
		break;
	}
	//case 0x3f: {
	//	state->cc.cy = ~state->cc.cy;
	//	break;
	//}
	case 0x40: { // MOV B,B
		break;
	}
	case 0x41: { // MOV B,C
		state->b = state->c;
		break;
	}
	case 0x45: { // MOV B,L
		state->b = state->l;
		break;
	}
	case 0x46: { // MOV B,M
		state->b = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x47: { // MOV B,A
		state->b = state->a;
		break;
	}
	case 0x48: { // MOV C,B
		state->c = state->b;
		break;
	}
	case 0x49: { // MOV C,C
		break;
	}
	case 0x4a: { // MOV C,D
		state->c = state->d;
		break;
	}
	case 0x4c: { // MOV C,H
		state->c = state->h;
		break;
	}
	case 0x4e: { // MOV C,M
		state->c = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x4f: {
		state->c = state->a;
		break;
	}
	case 0x51: { // MOV D,C
		state->d = state->c;
		break;
	}
	case 0x56: { // MOV D,M
		state->d = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x57: { // MOV D,A
		state->d = state->a;
		break;
	}
	case 0x5e: { // MOV E,M
		state->e = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x5f: { // MOV E,A
		state->e = state->a;
		break;
	}
	case 0x61: { // MOV H, C
		state->h = state->c;
		break;
	}
	case 0x65: { // MOV H,L
		state->h = state->l;
	}
	case 0x66: { // MOV H,M
		state->h = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x67: { // MOV H,A
		state->h = state->a;
		break;
	}
	case 0x68: { // MOV L, B
		state->l = state->b;
		break;
	}
	case 0x69: { // MOV L, C
		state->l = state->c;
		break;
	}
	case 0x6f: { // MOV L,A
		state->l = state->a;
		break;
	}
	case 0x70: { // MOV M,B
		write_to_register_pair_address(state, HL, state->b);
		break;
	}
	case 0x71: { // MOV M,C
		write_to_register_pair_address(state, HL, state->c);
		break;
	}
	case 0x77: { // MOV M,A
		write_to_register_pair_address(state, HL, state->a);
		break;
	}
	case 0x78: { // MOV A,B
		state->a = state->b;
		break;
	}
	case 0x79: {
		state->a = state->c;
		break;
	}
	case 0x80: { // ADD B
		uint16_t res = (uint16_t)state->a + (uint16_t)state->b;
		set_flags(state, res);
		state->a = res & 0xff;
		break;
	}
	case 0x7a: { // MOV A,D
		state->a = state->d;
		break;
	}
	case 0x7b: { // MOV A,E
		state->a = state->e;
		break;
	}
	case 0x7c: { // MOV A,H
		state->a = state->h;
		break;
	}
	case 0x7d: { // MOV A,L
		state->a = state->l;
		break;
	}
	case 0x7e: { // MOV A,M
		state->a = read_from_register_pair_address(state, HL);
		break;
	}
	case 0x7f: { // MOV A,A
		break;
	}

	//case 0x80: {
	//	uint16_t res = (uint16_t)state->a + (uint16_t)state->b;
	//	value_to_flags(state, res);
	//	state->a = res;
	//	break;
	//}

	case 0x81: {
		uint16_t res = (uint16_t)state->a + (uint16_t)state->c;
		set_flags(state, res);
		state->a = res & 0xff;
		break;
	}
	case 0x85: { // ADD L
		uint16_t res = (uint16_t)state->a + (uint16_t)state->l;
		set_flags(state, res);
		state->a = res & 0xff;
		break;
	}
	case 0x86: { // ADD M
		uint16_t res = (uint16_t)state->a + (uint16_t)read_from_register_pair_address(state, HL);
		set_flags(state, res);
		state->a = res & 0xff;
		break;
	}
	case 0x97: { // SUB A
		state->a = 0;
		set_flags(state, state->a);
		break;
	}
	case 0xa0: { // ANA B
		state->a &= state->b;
		set_flags(state, state->a);
		break;
	}
	case 0xa7: { // ANA A
		set_flags(state, state->a);
		break;
	}
	case 0xa6: { // ANA M
		state->a &= read_from_register_pair_address(state, HL);
		set_flags(state, state->a);
		break;
	}
	case 0xa8: { // XRA B
		state->a ^= state->b;
		set_flags(state, state->a);
		break;
	}
	case 0xaf: { // XRA A
		state->a = 0;
		set_flags(state, state->a);
		break;
	}
	case 0xb0: { // ORA B
		state->a |= state->b;
		set_flags(state, state->a);
		break;
	}
	case 0xb4: { // ORA H
		state->a |= state->h;
		set_flags(state, state->a);
		break;
	}
	case 0xb6: { // ORA M
		state->a |= read_from_register_pair_address(state, HL);
		set_flags(state, state->a);
		break;
	}
	case 0xb8: { // CMP B
		uint16_t res = (uint16_t)state->a - (uint16_t)state->b;
		set_flags(state, res);
		break;
	}
	case 0xbc: { // CMP H
		uint16_t compared = (uint16_t)state->a - (uint16_t)state->h;
		set_flags(state, compared);
		break;
	}
	case 0xbe: { // CMP M
		uint16_t compared = (uint16_t)state->a - (uint16_t)read_from_register_pair_address(state, HL);
		set_flags(state, compared);
		break;
	}
	case 0xc0: { // RNZ
		if (state->cc.z == false) {
			ret_opcode(state);
		}
		break;
	}
	case 0xc1: { // POP B
		state->c = pop_byte(state);
		state->b = pop_byte(state);
		break;
	}
	case 0xc2: { // JNZ
		if (state->cc.z == false) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xc3: { // JMP
		jmp_opcode(state, opcode[1], opcode[2]);
		break;
	}
	case 0xc4: { // CNZ adr
		if (state->cc.z == false) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xc5: { // PUSH BC
		push_pair(state, BC);
		break;
	}
	case 0xc6: { // ADI
		uint16_t res = (uint16_t)state->a + (uint16_t)opcode[1];
		state->a = res & 0xff;
		set_flags(state, res);
		state->pc++;
		break;
	}
	//case 0xc7: { // RST

	//}
	case 0xc8: { // RZ
		if (state->cc.z == true) {
			ret_opcode(state);
		}
		break;
	}
	case 0xc9: { // RET
		ret_opcode(state);
		break;
	}
	case 0xca: { // JZ
		if (state->cc.z == true) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xcc: { // CZ adr
		if (state->cc.z == true) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xcd: { // CALL
		call_opcode(state, opcode[1], opcode[2]);
		break;
	}
	case 0xd0: { // RNC
		if (state->cc.cy == false) {
			ret_opcode(state);
		}
		break;
	}
	case 0xd1: { // POP D
		state->e = pop_byte(state);
		state->d = pop_byte(state);
		break;
	}
	case 0xd2: { // JNC adr
		if (state->cc.cy == false) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xd3: { // OUT
		cpu->outTask.writePort(opcode[1], state->a, cpu->outTask.context);
		state->pc++;
		break;
	}
	case 0xd4: { // CNC adr
		if (state->cc.cy == false) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xd5: { // PUSH D
		push_pair(state, DE);
		break;
	}
	case 0xd6: { // SUI D8
		uint8_t res = state->a - opcode[1];
		set_flagsZSP(state, res);
		state->cc.cy = state->a < opcode[1]; // Comparison is in 8 bit, so we calculate CY in this approach
		state->a = res;
		state->pc++;
		break;

	}
	case 0xd8: {
		if (state->cc.cy == true) {
			ret_opcode(state);
		}
		break;
	}
	case 0xda: { // JC adr
		if (state->cc.cy == true) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xdb: { // IN
		state->a =  cpu->inTask.readPort(opcode[1], cpu->inTask.context);
		state->pc++;
		break;
	}
	case 0xde: { // SBI D8
		uint16_t res = state->a - opcode[1] - state->cc.cy;
		set_flagsZSP(state, state->a);
		state->cc.cy = res > 0xff; // Subtraction result is in 8 bit, so we calculate CY in this approach
		state->a = res & 0xff;
		state->pc++;
		break;
	}
	case 0xe1: { // POP H
		state->l = pop_byte(state);
		state->h = pop_byte(state);
		break;
	}
	case 0xe3: { // XTHL
		uint8_t tmpH = state->h;
		uint8_t tmpL = state->l;

		state->l = state->memory[state->sp];
		state->h = state->memory[state->sp + 1];
		
		write_to_memory(state, state->sp, tmpL);
		write_to_memory(state, state->sp + 1, tmpH);
		break;
	}

	case 0xe5: { // PUSH H
		push_pair(state, HL);
		break;
	}
	case 0xe6: { // ANI D8
		state->a &= opcode[1];
		set_flags(state, state->a);
		state->pc++;
		break;
	}
	case 0xe9: { // PCHL
		state->pc = get_register_pair(state, HL);
		break;
	}
	case 0xea: { // JPE adr
		if (state->cc.p == true) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xeb: { // XCHG
		uint8_t tmpH = state->h;
		uint8_t tmpL = state->l;

		state->h = state->d;
		state->l = state->e;

		state->d = tmpH;
		state->e = tmpL;
		break;
	}
	case 0xf1: { // POP PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		byte_to_flags(state, pop_byte(state)); // TODO: validate that this implemenation is correct. In the guide code he is not doing this line
		state->a = pop_byte(state);
		break;
	}
	case 0xf3: { // DI
		state->interrupt_enable = false;
		break;
	}
	case 0xf5: { // PUSH PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		push(state, flags_to_byte(state), state->a); // TODO: validate that this implemenation is correct. In the guide code he is not doing this line
		break;
	}

	case 0xf6: { // ORI D*8
		state->a |= opcode[1];
		set_flags(state, state->a);
		state->pc++;
		break;

	}
	//case 0xf9: { // SPHL
	//	uint8_t tempL = state->l;
	//	uint8_t tempH = state->h;
	//	state->l = state->memory[state->sp];
	//	state->h = state->memory[state->sp + 1];

	//	write_to_memory(state, state->sp, tempL);
	//	write_to_memory(state, state->sp + 1, tempH);

	//	break;
	//}
	case 0xfa: { // JM adr
		if (state->cc.s == true) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;

	}
	case 0xfb: { // EI
		state->interrupt_enable = true;
		break;
	}
	case 0xfe: { // CPI D*
		uint8_t sub = state->a - opcode[1];
		set_flagsZSP(state, sub);
		state->cc.cy = state->a < opcode[1]; // Subtraction is in 8 bit, so we calculate CY in this approach
		state->pc++;
		break;
	}
	default:
		state->pc--;
		unimplemented_instruction(state);
	}
	return 0;
}