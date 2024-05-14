#include "Emulate8080.h"

bool Parity(int value, int size) {
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
void byteToFlags(State8080* state, uint8_t flags) {
	state->cc.cy = flags & 1;
	state->cc.p = (flags >> 2) & 1;
	state->cc.ac = (flags >> 4) & 1;
	state->cc.z = (flags >> 6) & 1;
	state->cc.s = (flags >> 7) & 1;
}

uint8_t flagsToByte(State8080* state) {
	uint8_t flags = state->cc.cy | state->cc.p << 2 | state->cc.ac << 4 | state->cc.z << 6 | state->cc.s << 7;
	return flags;
}

int counter = 0;

void printState(State8080* state) {
	printf("\tcounter=%d, C=%d,P=%d,S=%d,Z=%d\n", counter++, state->cc.cy, state->cc.p,
		state->cc.s, state->cc.z);
	printf("\tAF: $%02x%02x BC: $%02x%02x DE: $%02x%02x HL $%02x%02x PC %04x SP %04x\nInstruction %02x\n",
		state->a, flagsToByte(state), state->b, state->c, state->d,
		state->e, state->h, state->l, state->pc, state->sp, state->memory[state->pc]);
}

void UnimplementedInstruction(State8080* state)
{
	printf("Error: Unimplemented instruction $%02x\n", state->memory[state->pc]); // TODO: add information from the state.
	printState(state);
	exit(EXIT_FAILURE);
}

State8080* initState() {
	State8080* state = (State8080*)calloc(1, sizeof(State8080));

	state->memory = (uint8_t*)calloc(20000, sizeof(uint8_t)); // TODO: check the correct size;
	return state;
}

void freeState(State8080* state) {
	//free(state->memory);
	free(state);
}




int Emulate8080Op(State8080* state)
{
	unsigned char* opcode = &state->memory[state->pc];
	printState(state);

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
		state->pc += 1;
		break;
	}
	case 0x02: {
		uint16_t bc = (state->b << 8) | state->c;
		state->memory[bc] = state->a;
		break;
	}
	case 0x05: {
		state->b = state->b - 1;
		state->cc.z = state->b == 0;
		state->cc.s = (state->b >> 7);
		state->cc.p = Parity(state->b, 8);
		break;
	}
	case 0x06: {
		state->b = opcode[1];
		state->pc += 1;
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
		state->h = (combined & 0xff00 >> 8);
		state->l = combined & 0xff;
		state->cc.cy = (combined & 0xffff0000) > 0;
		break;
	}
	case 0x0a: {
		uint16_t BC = (state->b << 8) | state->c;
		state->a = state->memory[BC];
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
		state->e = DE & 0x0f;
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
		state->l = HL & 0x0f;
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
	case 0x36: {
		uint16_t HL = (state->h << 8) | state->l;
		state->memory[HL] = opcode[1];
		break;
	}
	case 0x37: {
		state->cc.cy = 1;
		break;
	}
	case 0x3f: {
		state->cc.cy = ~state->cc.cy;
		break;
	}
	case 0x77: {
		uint16_t HL = (state->h << 8) | state->l;
		state->memory[HL] = state->a;
		break;
	}

	case 0x7c: {
		state->a = state->h;
		break;
	}
	case 0x80: {
		uint16_t res = (uint16_t)state->a + (uint16_t)state->b;
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 1;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = Parity(res, 8);
		break;
	}

	case 0x81: {
		uint16_t res = (uint16_t)state->a + (uint16_t)state->c;
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 1;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = Parity(res, 8);
		break;
	}
	case 0x86: {
		uint32_t HL = (state->h << 8) | state->l;
		uint16_t res = (uint16_t)state->a + (uint16_t)state->memory[HL];
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 1;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = Parity(res, 8);
		break;
	}
	case 0xc1: { // POP B
		state->c = state->memory[state->sp];
		state->b = state->memory[state->sp + 1];
		state->sp += 2;
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
	case 0xc3: {
		state->pc = (opcode[2] << 8) | opcode[1];
		break;
	}
	case 0xc5: { // PUHS B
		state->memory[state->sp - 1] = state->b;
		state->memory[state->sp - 2] = state->c;
		state->sp -= 2;
		break;
	}
	case 0xc6: {
		uint16_t res = (uint16_t)state->a + (uint16_t)opcode[1];
		state->cc.z = (res & 0xff) == 0;
		state->cc.s = (res & 0x80) == 1;
		state->cc.cy = res > 0xff;
		res &= 0xff;
		state->a = res;
		state->cc.p = Parity(res, 8);
		break;
	}

			 //case 0xc7: { // RST

			 //}
	case 0xc9: { // RET
		state->pc = (state->memory[state->sp + 1] << 8) | state->memory[state->sp];
		state->sp += 2;
		break;
	}
	case 0xcd: {
		uint16_t ret = state->pc + 2;
		state->memory[state->sp - 1] = (ret >> 8) & 0xff;
		state->memory[state->sp - 2] = ret & 0xff;
		state->sp -= 2;
		state->pc = (opcode[2] << 8) | opcode[1];
		break;
	}

	case 0xd3: { // Will come back for it, for now just skip the data byte
		state->pc++;
		break;
	}
	case 0xdb: { // Will come back for it, for now just skip the data byte
		state->pc++;
		break;
	}
	case 0xe3: {
		state->sp = (state->h << 8) | state->l;

		break;
	}
	case 0xe6: {
		state->a &= opcode[1];
		state->cc.z = state->a == 0;
		state->cc.s = (state->a >> 7);
		state->cc.p = Parity(state->a, 8);
		state->cc.cy = 0;
		state->pc++;
		break;
	}

	case 0xe9: {
		state->pc = (state->h << 8) | state->l;
		break;
	}
	case 0xf1: { // POP PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		byteToFlags(state, state->memory[state->sp]);

		state->a = state->memory[state->sp + 1];
		state->sp += 2;
		break;
	}
	case 0xf5: { // PUSH PSW TODO: validate that the flag bit mapping is correct. I used the data book mapping, but the emulator101 guide uses a different one for some reason.
		state->memory[state->sp - 1] = state->a;
		state->memory[state->sp - 2] = (state->cc.cy) | (state->cc.p << 2) | (state->cc.ac << 4) | (state->cc.z << 6) | (state->cc.s << 7);
		state->sp -= 2;
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
	case 0xfe: {
		uint8_t sub = state->a - opcode[1];
		state->cc.z = sub == 0;
		state->cc.s = (sub & 0x80) >> 7;
		state->cc.p = Parity(sub, 8);
		state->cc.cy = sub < 0;

		state->pc++;
		break;
	}


	default:
		state->pc--;
		UnimplementedInstruction(state);

	}


	return 0;
}