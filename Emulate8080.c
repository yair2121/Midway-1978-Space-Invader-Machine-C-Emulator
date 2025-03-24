#include "Emulate8080.h"

static bool parity(int value, int size) {
	value = (value & ((1 << size) - 1));
	int activeBitCount = 0;
	for (int i = 0; i < size; i++) {
		activeBitCount += value & 1;
		value = value >> 1;
	}
	return activeBitCount % 2 == 0;
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
	uint8_t flags = state->cc.cy | state->cc.p << 2 | state->cc.ac << 4 | state->cc.z << 6 | state->cc.s << 7;
	return flags;
}

static void print_state(State8080* state) {
	printf("\n\n\n");
	disassemble_8080_op(state->memory, state->pc);
	printf("\C=%d,P=%d,S=%d,Z=%d\n", state->cc.cy, state->cc.p,
		state->cc.s, state->cc.z);
	printf("\tAF: $%02x%02x BC: $%02x%02x DE: $%02x%02x HL $%02x%02x PC %04x SP %04x\n\n\n",
		state->a, flags_to_byte(state), state->b, state->c, state->d,
		state->e, state->h, state->l, state->pc, state->sp);
}

static void unimplemented_instruction(State8080* state)
{
	printf("Error: Unimplemented instruction $%02x\n", state->memory[state->pc]); // TODO: add information from the state.
	print_state(state);
	exit(EXIT_FAILURE);
}

Cpu8080* init_cpu_state(size_t bufferSize, uint8_t* codeBuffer, size_t memorySize) {
	Cpu8080* cpu = (Cpu8080*)calloc(1, sizeof(Cpu8080));

	cpu->state = (State8080*)calloc(1, sizeof(State8080));

	cpu->state->memory = (uint8_t*)calloc(memorySize, sizeof(uint8_t));
	memcpy_s(cpu->state->memory, memorySize, codeBuffer, bufferSize);

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
	free(cpu);
}

static void push(State8080* state, uint8_t arg1, uint8_t arg2) {
	state->memory[state->sp - 1] = arg1;
	state->memory[state->sp - 2] = arg2;

	state->sp -= 2;
}

static uint8_t pop_byte(State8080* state) {
	uint8_t result = state->memory[state->sp];
	state->sp++;
	return result;
}

void GenerateInterrupt(State8080* state, int interrupt_num) {
	//perform "PUSH PC"
	push(state, (state->pc & 0xFF00) >> 8, (state->pc & 0xff));


	//Set the PC to the low memory vector.    
	//This is identical to an "RST interrupt_num" instruction.    
	state->pc = 8 * interrupt_num;
}


int emulate_8080_op(Cpu8080* cpu) {
	State8080* state = cpu->state;
	uint8_t* opcode = &state->memory[state->pc];
	
	print_state(state);
	state->pc += 1;

	switch (*opcode)
	{
	case 0x00:
	case 0x08:
	case 0x18:
	case 0x28:
	case 0x38:
	case 0xcb:
	case 0xd9:
	case 0xdd:
	case 0xed:
	case 0xfd:
		break;

	case 0x01: {
		state->b = opcode[2];
		state->c = opcode[1];
		state->pc += 2;
		break;
	}
	case 0x02: {
		uint16_t bc = (state->b << 8) | state->c;
		state->memory[bc] = state->a;
		break;
	}
	case 0x05: {
		state->b--;
		state->cc.z = state->b == 0;
		state->cc.s = (state->b >> 7);
		state->cc.p = parity(state->b, 8);
		break;
	}
	case 0x06: {
		state->b = opcode[1];
		state->pc++;
		break;
	}
	case 0x07: {
		state->cc.cy = state->a >> 7;
		state->a = (state->a << 1) | state->cc.cy;
		break;
	}
	case 0x09: {
		uint32_t HL = (state->h << 8) | state->l;
		uint32_t BC = (state->b << 8) | state->c;
		uint32_t combined = HL + BC;
		state->h = (combined & 0xff00) >> 8;
		state->l = combined & 0xff;
		state->cc.cy = (combined & 0xffff0000) > 0;
		break;
	}
	case 0x0a: {
		uint16_t BC = (state->b << 8) | state->c;
		state->a = state->memory[BC];
		break;
	}
	case 0x0d: { // DCR C
		state->c--;
		state->cc.z = state->c == 0;
		state->cc.s = state->c >> 7;
		state->cc.p = parity(state->c, 8);
		break;
	}
	case 0x0e: {
		state->c = opcode[1];
		state->pc++;
		break;
	}
	case 0x0f: { // RRC
		state->cc.cy = state->a & 1;
		state->a = (state->a << 7) | state->a >> 1;
		break;
	}
	case 0x11: {
		state->e = opcode[1];
		state->d = opcode[2];
		state->pc += 2;
		break;
	}
	case 0x13: {
		uint16_t DE = (state->d << 8) | state->e;
		DE++;
		state->d = DE >> 8;
		state->e = DE & 0xff;
		break;
	}
	case 0x19: { // DAD H
		uint32_t HL = (state->h << 8) | state->l;
		uint32_t DE = (state->d << 8) | state->e;
		HL += DE;
		state->h = HL >> 8;
		state->l = HL & 0xff;
		state->cc.cy = (HL & 0xffff0000) != 0;
		break;
	}
	case 0x1a: {
		uint16_t DE = (state->d << 8) | state->e;
		state->a = state->memory[DE];
		break;
	}

	case 0x1f: {
		state->cc.cy = state->a & 1;
		state->a = (state->a & 0x80) | state->a >> 1;
		break;
	}
	case 0x21: {
		state->l = opcode[1];
		state->h = opcode[2];
		state->pc += 2;
		break;
	}
	case 0x23: {
		uint16_t HL = (state->h << 8) | state->l;
		HL++;
		state->h = HL >> 8;
		state->l = HL & 0xff;
		break;
	}
	case 0x26: {
		state->h = opcode[1];
		state->pc++;
		break;
	}
	case 0x29: { // DAD H : HL *= 2
		uint32_t HL = (state->h << 8) | state->l;
		HL *= 2;
		state->h = HL >> 8;
		state->l = HL & 0xff;
		state->cc.cy = (HL & 0xffff0000) != 0;
		break;
	}
	case 0x2f: {
		state->a = ~state->a;
		break;
	}
	case 0x31: { // TODO: validate
		state->sp = (opcode[2] << 8) | opcode[1];
		state->pc += 2;
		break;
	}
	case 0x32: { // STA adr
		uint16_t address = (opcode[2] << 8) | opcode[1];
		state->memory[address] = state->a;
		state->pc += 2;
		break;
	}
	case 0x36: {
		uint16_t HL = (state->h << 8) | state->l;
		state->memory[HL] = opcode[1];
		state->pc++;
		break;
	}
	case 0x37: {
		state->cc.cy = 1;
		break;
	}
	case 0x3a: { // LDA adr
		uint16_t address = (opcode[2] << 8) | opcode[1];
		state->a = state->memory[address];
		state->pc += 2;
		break;
	}
	case 0x3e: {
		state->a = opcode[1];
		state->pc++;
		break;
	}
	case 0x3f: {
		state->cc.cy = ~state->cc.cy;
		break;
	}
	case 0x56: {
		uint16_t HL = (state->h << 8) | state->l;
		state->d = state->memory[HL];
		break;
	}
	case 0x5e: {
		uint16_t HL = (state->h << 8) | state->l;
		state->e = state->memory[HL];
		break;
	}
	case 0x66: {
		uint16_t HL = (state->h << 8) | state->l;
		state->h = state->memory[HL];
		break;
	}
	case 0x6f: {
		state->l = state->a;
		break;
	}
	case 0x77: {
		uint16_t HL = (state->h << 8) | state->l;
		state->memory[HL] = state->a;
		break;
	}
	case 0x7a: {
		state->a = state->d;
		break;
	}
	case 0x7b: {
		state->a = state->e;
		break;
	}
	case 0x7c: {
		state->a = state->h;
		break;
	}
	case 0x7e: {
		uint16_t HL = (state->h << 8) | state->l;
		state->a = state->memory[HL];
		break;
	}
	case 0x80: {
		uint16_t res = (uint16_t)state->a + (uint16_t)state->b;
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 0x80;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = parity(res, 8);
		break;
	}

	case 0x81: {
		uint16_t res = (uint16_t)state->a + (uint16_t)state->c;
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 0x80;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = parity(res, 8);
		break;
	}
	case 0x86: {
		uint16_t HL = (state->h << 8) | state->l;
		uint16_t res = (uint16_t)state->a + (uint16_t)state->memory[HL];
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 0x80;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = parity(res, 8);
		break;
	}
	case 0xa7: { // ANA A
		state->a &= state->a;

		state->cc.z = state->a == 0;
		state->cc.s = state->a >> 7;
		state->cc.p = parity(state->a, 8);
		state->cc.cy = 0;
		break;
	}
	case 0xaf: { // TODO: extract to function
		state->a ^= state->a;

		state->cc.z = 1;
		state->cc.s = 0;
		state->cc.p = 1;
		state->cc.cy = 0;
		break;

	}
	case 0xc1: { // POP B
		state->c = pop_byte(state);
		state->b = pop_byte(state);
		break;
	}
	case 0xc2: { // JNZ
		if (state->cc.z == 0) {
			state->pc = (opcode[2] << 8) | opcode[1];
		}
		else {
			state->pc += 2;
		}
		break;
	}
	case 0xc3: { // JMP
		state->pc = (opcode[2] << 8) | opcode[1];
		break;
	}
	case 0xc5: { // PUSH B
		push(state, state->b, state->c);
		break;
	}
	case 0xc6: {
		uint16_t res = (uint16_t)state->a + (uint16_t)opcode[1];
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 0x80;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = parity(res, 8);

		state->pc++;
		break;
	}

			 //case 0xc7: { // RST

			 //}
	case 0xc9: { // RET
		state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
		state->sp += 2;
		break;
	}
	case 0xcd: { // CALL
		uint16_t ret = state->pc + 2;
		state->memory[state->sp - 1] = (ret >> 8) & 0xff;
		state->memory[state->sp - 2] = ret & 0xff;
		state->sp -= 2;
		state->pc = (opcode[2] << 8) | opcode[1];
		break;
	}
	case 0xd1: { // POP D
		state->e = pop_byte(state);
		state->d = pop_byte(state);
		break;
	}
	case 0xd3: { // OUT
		cpu->outTask.writePort(opcode[1], state->a, cpu->outTask.context);
		state->pc++;
		break;
	}
	case 0xd5: { // PUSH D
		push(state, state->d, state->e);
		break;
	}
	case 0xdb: { // IN
		state->a = cpu->inTask.readPort(opcode[1], cpu->inTask.context);
		state->pc++;
		break;
	}
	case 0xe1: {
		state->l = pop_byte(state);
		state->h = pop_byte(state);
		break;
	}
	case 0xe3: {
		state->sp = (state->h << 8) | state->l;

		break;
	}

	case 0xe5: { // PUSH H
		push(state, state->h, state->l);
		break;
	}
	case 0xe6: {
		state->a &= opcode[1];
		state->cc.z = state->a == 0;
		state->cc.s = (state->a >> 7);
		state->cc.p = parity(state->a, 8);
		state->cc.cy = 0;
		state->pc++;
		break;
	}

	case 0xe9: {
		state->pc = (state->h << 8) | state->l;
		break;
	}

	case 0xeb: {
		uint8_t tmpH = state->h;
		uint8_t tmpL = state->l;
		state->h = state->d;
		state->l = state->e;
		state->d = tmpH;
		state->e = tmpL;
		break;
	}
	case 0xf1: { // POP PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		byte_to_flags(state, pop_byte(state));
		state->a = pop_byte(state);
		break;
	}
	case 0xf3: { // DI
		state->interrupt_enable = false;
		break;
	}
	case 0xf5: { // PUSH PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		push(state, state->a, flags_to_byte(state));
		break;
	}

	case 0xf9: { // SPHL
		uint8_t tempL = state->l;
		uint8_t tempH = state->h;

		state->l = state->memory[state->sp];
		state->h = state->memory[state->sp + 1];

		state->memory[state->sp] = tempL;
		state->memory[state->sp + 1] = tempH;

		break;
	}
	case 0xfb: { // EI
		state->interrupt_enable = true;
		break;
	}
	case 0xfe: {
		uint8_t sub = state->a - opcode[1];
		state->cc.z = sub == 0;
		state->cc.s = (sub & 0x80) >> 7;
		state->cc.p = parity(sub, 8);
		state->cc.cy = state->a < opcode[1];

		state->pc++;
		break;
	}


	default:
		state->pc--;
		unimplemented_instruction(state);

	}


	return 0;
}