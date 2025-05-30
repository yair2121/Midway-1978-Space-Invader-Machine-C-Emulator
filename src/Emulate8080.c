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

static void ADD(State8080* state, uint16_t value) {
	uint16_t result = (uint16_t)state->a + value;
	state->a = result & 0xff;
	set_flags(state, result);
}

static void ADC(State8080* state, uint16_t value) {
	uint16_t result = (uint16_t)state->a + value + state->cc.cy;
	state->a = result & 0xff;
	set_flags(state, result);
}

static void SUB(State8080* state, uint16_t value) {
	uint16_t result = (uint16_t)state->a - value;
	state->a = result & 0xff;
	set_flags(state, result);
}

static void SBB(State8080* state, uint16_t value) {
	uint16_t result = (uint16_t)state->a - value - state->cc.cy;
	state->a = result & 0xff;
	set_flags(state, result);
}

static void ANA(State8080* state, uint8_t value) {
	state->a = state->a & value;
	set_flags(state, state->a);
}

static void XRA(State8080* state, uint8_t value) {
	state->a = state->a ^ value;
	set_flags(state, state->a);
}
static void ORA(State8080* state, uint8_t value) {
	state->a = state->a | value;
	set_flags(state, state->a);
}
static void CMP(State8080* state, uint16_t value) {
	uint16_t result = (uint16_t)state->a - value;
	set_flags(state, result);
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
	case 0x02: {// STAX B
		write_to_register_pair_address(state, BC, state->a); 
		break;
	}
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
	case 0x0b: { // INR C
		uint16_t bc = get_register_pair(state, BC);
		set_register_pair(state, BC, bc - 1);
		break;
	}
	case 0x0c: { // INR C
		state->c++;
		set_flagsZSP(state, state->c);
		break;
	}
	case 0x0d: { // DCR C
		state->c--;
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
	case 0x12: { // STAX D 
		write_to_register_pair_address(state, DE, state->a);
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
	case 0x17: { // RAL
		uint8_t prevCY = state->cc.cy;
		uint8_t prevBit7 = (state->a & 0x80) >> 7;
		state->a = (state->a << 1) | state->cc.cy;
		state->cc.cy = prevBit7;
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
	case 0x1b: { // DCX D
		uint16_t de = get_register_pair(state, DE);
		set_register_pair(state, DE, de - 1);
	}
	case 0x1c: { // INR E
		state->e++;
		set_flagsZSP(state, state->e);
	}
	case 0x1d: { // DCR E
		state->e--;
		set_flagsZSP(state, state->e);
	}
	case 0x1e: { // MVI E,D8
		state->e = opcode[1];
		state->pc++;
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
	case 0x24: { // INR H
		state->h++;
		set_flagsZSP(state, state->h);
	}
	case 0x25: { // DCR H
		state->h--;
		set_flagsZSP(state, state->h);
	}
	case 0x26: { // MVI H,D8
		state->h = opcode[1];
		state->pc++;
		break;
	}
	case 0x27: { // DAA
		if ((state->a & 0x0f) > 9) { // Space Invader does not use the AC flag, so I ignored it in this comparasion. For full emulation It needs to be considered here according to the data book
			state->a += 0x06;
		}

		if (state->cc.cy || (state->a & 0xf0) > 9) {
			uint8_t a_highNibble = (state->a & 0xf0) >> 4;
			a_highNibble += 6;
			state->a &= 0x0f | (a_highNibble << 4);
		}
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
	case 0x2d: { // DCR L
		state->l--;
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
	case 0x33: { // INX SP
		state->sp += 1;
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
	case 0x39: { // DAD SP
		uint32_t hl = get_register_pair(state, HL);
		uint32_t combined = hl + (uint32_t) state->sp;
		set_register_pair(state, HL, combined & 0xffff);
		state->cc.cy = (combined & 0xffff0000) != 0;
		break;
	}
	case 0x3a: { // LDA adr
		uint16_t address = pair_to_address(opcode[1], opcode[2]);
		state->a = state->memory[address];
		state->pc += 2;
		break;
	}
	case 0x3b: { // DCX SP
		state->sp -= 1;
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
	case 0x3f: { // 0x3f
		state->cc.cy = ~state->cc.cy;
		break;
	}
	case 0x40: { // MOV B,B
		break;
	}
	case 0x41: { // MOV B,C
		state->b = state->c;
		break;
	}
	case 0x42: { // MOV B,D
		state->b = state->d;
		break;
	}
	case 0x43: { // MOV B,E
		state->b = state->e;
		break;
	}
	case 0x44: { // MOV B,H
		state->b = state->h;
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
	case 0x4b: { // MOV C,E
		state->c = state->e;
		break;
	}
	case 0x4c: { // MOV C,H
		state->c = state->h;
		break;
	}
	case 0x4d: { // MOV C,L
		state->c = state->l;
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
	case 0x50: { // MOV D,B
		state->d = state->b;
		break;
	}
	case 0x51: { // MOV D,C
		state->d = state->c;
		break;
	}
	case 0x52: { // MOV D,D
		break;
	}
	case 0x53: { // MOV D,E
		state->d = state->e;
		break;
	}
	case 0x54: { // MOV D,H
		state->d = state->h;
		break;
	}
	case 0x55: { // MOV D,L
		state->d = state->l;
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
	case 0x58: { // MOV E,B
		state->e = state->b;
		break;
	}
	case 0x59: { // MOV E,C
		state->e = state->c;
		break;
	}
	case 0x5a: { // MOV E,D
		state->e = state->d;
		break;
	}
	case 0x5b: { // MOV E,E
		break;
	}
	case 0x5c: { // MOV E,H
		state->e = state->h;
		break;
	}
	case 0x5d: { // MOV E,L
		state->e = state->l;
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
	case 0x60: { // MOV H,B
		state->h = state->b;
		break;
	}
	case 0x61: { // MOV H, C
		state->h = state->c;
		break;
	}
	case 0x62: { // MOV H,D
		state->h = state->d;
		break;
	}
	case 0x63: { // MOV H,E
		state->h = state->e;
		break;
	}
	case 0x64: { // MOV H,H
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
	case 0x6a: { // MOV L, D
		state->l = state->d;
		break;
	}
	case 0x6b: { // MOV L, E
		state->l = state->e;
		break;
	}
	case 0x6c: { // MOV L, H
		state->l = state->h;
		break;
	}
	case 0x6d: { // MOV L, L
		break;
	}
	case 0x6e: { // MOV L, M
		state->l = read_from_register_pair_address(state, HL);
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
	case 0x72: { // MOV M,D
		write_to_register_pair_address(state, HL, state->d);
		break;
	}
	case 0x73: { // MOV M,E
		write_to_register_pair_address(state, HL, state->e);
		break;
	}
	case 0x74: { // MOV M,H
		write_to_register_pair_address(state, HL, state->h);
		break;
	}
	case 0x75: { // MOV M,L
		write_to_register_pair_address(state, HL, state->l);
		break;
	}
	case 0x76: { // HLT
		break; // TODO: check if something need to be done here
	}
	case 0x77: { // MOV M,A
		write_to_register_pair_address(state, HL, state->a);
		break;
	}
	case 0x78: { // MOV A,B
		state->a = state->b;
		break;
	}
	case 0x79: { // MOV A,C
		state->a = state->c;
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
	case 0x80: { // ADD B
		ADD(state, state->b);
		break;
	}
	case 0x81: { // ADD C
		ADD(state, state->c);
		break;
	}
	case 0x82: { // ADD D
		ADD(state, state->d);
		break;
	}
	case 0x83: { // ADD E
		ADD(state, state->e);
		break;
	}
	case 0x84: { // ADD H
		ADD(state, state->h);
		break;
	}
	case 0x85: { // ADD L
		ADD(state, state->l);
		break;
	}
	case 0x86: { // ADD M
		ADD(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0x87: { // ADD A
		ADD(state, state->a);
		break;
	}
	case 0x88: { // ADC B
		ADC(state, state->b);
		break;
	}
	case 0x89: { // ADC C
		ADC(state, state->c);
		break;
	}
	case 0x8a: { // ADC D
		ADC(state, state->d);
		break;
	}
	case 0x8b: { // ADC E
		ADC(state, state->e);
		break;
	}
	case 0x8c: { // ADC H
		ADC(state, state->h);
		break;
	}
	case 0x8d: { // ADC L
		ADC(state, state->l);
		break;
	}
	case 0x8e: { // ADC M
		ADC(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0x8f: { // ADC A
		ADC(state, state->a);
		break;
	}
	case 0x90: { // SUB B
		SUB(state, state->b);
		break;
	}
	case 0x91: { // SUB C
		SUB(state, state->c);
		break;
	}
	case 0x92: { // SUB D
		SUB(state, state->d);
		break;
	}
	case 0x93: { // SUB E
		SUB(state, state->e);
		break;
	}
	case 0x94: { // SUB H
		SUB(state, state->h);
		break;
	}
	case 0x95: { // SUB L
		SUB(state, state->l);
		break;
	}
	case 0x96: { // SUB M
		SUB(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0x97: { // SUB A
		SUB(state, state->a);
		break;
	}
	case 0x98: { // SBB B
		SBB(state, state->b);
		break;
	}
	case 0x99: { // SBB C
		SBB(state, state->c);
		break;
	}
	case 0x9a: { // SBB D
		SBB(state, state->d);
		break;
	}
	case 0x9b: { // SBB E
		SBB(state, state->e);
		break;
	}
	case 0x9c: { // SBB H
		SBB(state, state->h);
		break;
	}
	case 0x9d: { // SBB L
		SBB(state, state->l);
		break;
	}
	case 0x9e: { // SBB M
		SBB(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0x9f: { // SBB A
		SBB(state, state->a);
		break;
	}
	case 0xa0: { // ANA B
		ANA(state, state->b);
		break;
	}
	case 0xa1: { // ANA C
		ANA(state, state->c);
		break;
	}
	case 0xa2: { // ANA D
		ANA(state, state->d);
		break;
	}
	case 0xa3: { // ANA E
		ANA(state, state->e);
		break;
	}
	case 0xa4: { // ANA H
		ANA(state, state->h);
		break;
	}
	case 0xa5: { // ANA L
		ANA(state, state->l);
		break;
	}
	case 0xa6: { // ANA M
		ANA(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0xa7: { // ANA A
		ANA(state, state->a);
		break;
	}
	case 0xa8: { // XRA B
		XRA(state, state->b);
		break;
	}
	case 0xa9: { // XRA C
		XRA(state, state->c);
		break;
	}
	case 0xaa: { // XRA D
		XRA(state, state->d);
		break;
	}
	case 0xab: { // XRA E
		XRA(state, state->e);
		break;
	}
	case 0xac: { // XRA H
		XRA(state, state->h);
		break;
	}
	case 0xad: { // XRA L
		XRA(state, state->l);
		break;
	}
	case 0xae: { // XRA M
		XRA(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0xaf: { // XRA A
		XRA(state, state->a);
		break;
	}
	case 0xb0: { // ORA B
		ORA(state, state->b);
		break;
	}
	case 0xb1: { // ORA C
		ORA(state, state->c);
		break;
	}
	case 0xb2: { // ORA D
		ORA(state, state->d);
		break;
	}
	case 0xb3: { // ORA E
		ORA(state, state->e);
		break;
	}
	case 0xb4: { // ORA H
		ORA(state, state->h);
		break;
	}
	case 0xb5: { // ORA L
		ORA(state, state->l);
		break;
	}
	case 0xb6: { // ORA M
		ORA(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0xb7: { // ORA A
		ORA(state, state->a);
		break;
	}
	case 0xb8: { // CMP B
		CMP(state, state->b);
		break;
	}
	case 0xb9: { // CMP C
		CMP(state, state->c);
		break;
	}
	case 0xba: { // CMP D
		CMP(state, state->d);
		break;
	}
	case 0xbc: { // CMP H
		CMP(state, state->h);
		break;
	}
	case 0xbd: { // CMP L
		CMP(state, state->l);
		break;
	}
	case 0xbe: { // CMP M
		CMP(state, read_from_register_pair_address(state, HL));
		break;
	}
	case 0xbf: { // CMP A
		CMP(state, state->a);
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
	case 0xc7: { // RST
		call_opcode(state, 0, 0);
		break;
	}
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
	case 0xce: { //ACI D&
		ADC(state, opcode[1]);
		state->pc++;
		break;
	}
	case 0xcf: { // RST 1
		call_opcode(state, 0x08, 0);
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
	case 0xd7: { // RST 2
		call_opcode(state, 0x10, 0);
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
	case 0xdc: { // CC adr
		if (state->cc.cy == true) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
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
	case 0xdf: { // RST 3
		call_opcode(state, 0x18, 0);
		break;
	}
	case 0xe0: { // RPO
		if (state->cc.p == false) {
			ret_opcode(state);
		}
		break;
	}
	case 0xe1: { // POP H
		state->l = pop_byte(state);
		state->h = pop_byte(state);
		break;
	}
	case 0xe2: { // JPO
		if (state->cc.p == false) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
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
	case 0xe4: { // CPO adr
		if (state->cc.p == false) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
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
	case 0xe7: { // RST 4
		call_opcode(state, 0x20, 0);
		break;
	}
	case 0xe8: { // RPE
		if (state->cc.p == true) {
			ret_opcode(state);
		}
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
	case 0xec: { // CPE adr
		if (state->cc.p == true) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xee: { // XRI D8
		XRA(state, opcode[1]);
		state->pc++;
		break;
	}
	case 0xef: { // RST 5
		call_opcode(state, 0x28, 0);
		break;
	}
	case 0xf0: { // RP
		if (state->cc.s == false) {
			ret_opcode(state);
		}
		break;
	}
	case 0xf1: { // POP PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		byte_to_flags(state, pop_byte(state)); // TODO: validate that this implemenation is correct. In the guide code he is not doing this line
		state->a = pop_byte(state);
		break;
	}
	case 0xf2: {
		if (state->cc.s == false) {
			jmp_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xf3: { // DI
		state->interrupt_enable = false;
		break;
	}
	case 0xf4: { // CPE adr
		if (state->cc.s == false) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
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
	case 0xf7: { // RST 6
		call_opcode(state, 0x30, 0);
		break;
	}
	case 0xf8: { // RM
		if (state->cc.s == true) {
			ret_opcode(state);
		}
		break;
	}
	case 0xf9: { // SPHL
		state->sp = get_register_pair(state, HL);
		break;
	}
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
	case 0xfc: { // CM D8
		if (state->cc.s == true) {
			call_opcode(state, opcode[1], opcode[2]);
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xfe: { // CPI D*
		uint8_t sub = state->a - opcode[1];
		set_flagsZSP(state, sub);
		state->cc.cy = state->a < opcode[1]; // Subtraction is in 8 bit, so we calculate CY in this approach
		state->pc++;
		break;
	}
	case 0xff: { // RST 7
		call_opcode(state, 0x38, 0);
		break;
	}
	default:
		state->pc--;
		unimplemented_instruction(state);
	}
	return 0;
} 