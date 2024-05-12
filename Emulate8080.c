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

void UnimplementedInstruction(State8080* state)
{
	printf("Error: Unimplemented instruction\n"); // TODO: add information from the state.
	exit(EXIT_FAILURE);
}

int Emulate8080Op(State8080* state)
{
	unsigned char* opcode = &state->memory[state->pc];
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
		uint32_t BC = (state->b << 8) | state->c;
		state->a = state->memory[BC];
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
	case 0xc2: {
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
	case 0xc9: {
		uint16_t pcHigh = state->memory[state->pc] << 8;
		uint16_t pcLow = state->memory[state->pc] & 0xff;
		state->pc = pcHigh | pcLow;
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


	default:
		state->pc--;
		UnimplementedInstruction(state);

	}

	return 0;
}