#include "Opcodes8080.h"

/// <summary>
/// Targets the following opcodes types which follow the same opcode layout: ADD, ADC, SUB, SBB, ANA, XRA, ORA, CMP.
/// The value to target is either an immediate value (if the opcode matches the immediate_opcode) or a value from a register (if the opcode is in the range defined by range_min).
/// </summary>
/// <param name="state"></param>
/// <param name="opcode"></param>
/// <param name="range_min"></param>
/// <param name="immediate_opcode"></param>
static uint16_t get_single_target_value(State8080 *state, uint8_t *opcode, uint8_t range_min, uint8_t immediate_opcode)
{
	uint16_t target_value;
	if (*opcode == immediate_opcode)
	{
		target_value = opcode[1];
	}
	else
	{
		GENERAL_REGISTER target_register = (*opcode - range_min); // Get the register index
		if (target_register == HL_GENERAL_REGISTER)
		{
			target_value = read_from_register_pair_address(state, HL);
		}
		else
		{
			target_value = state->general_register[target_register];
		}
	}

	return target_value;
}

void op_mov(State8080 *state, GENERAL_REGISTER source, GENERAL_REGISTER target)
{
	if (target == HL_GENERAL_REGISTER)
	{
		write_to_register_pair_address(state, HL, state->general_register[source]); // Write to memory at HL address // TODO: validate this
	}
	else if (source == HL_GENERAL_REGISTER)
	{
		state->general_register[target] = read_from_register_pair_address(state, HL);
	}
	else
	{
		state->general_register[target] = state->general_register[source];
	}
}

RANGE_WITH_HOLE_VALUE_SHOULD_HANDLE(MOV, MOV_END_RANGE, HLT);
void handle_mov(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{																			// MOV TARGET, SOURCE
	GENERAL_REGISTER source = (*opcode - MOV) % NUMBER_OF_OPCODE_REGISTERS; // Get the register index
	GENERAL_REGISTER target = (*opcode - MOV) / NUMBER_OF_OPCODE_REGISTERS; // Get the target register index

	op_mov(state, source, target);
}

DISCRETE_VALUE_SHOULD_HANDLE(NOP, 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x38, 0xcb, 0xd9, 0xdd, 0xed, 0xfd);
void handle_nop(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) {}

SINGLE_VALUE_SHOULD_HANDLE(HLT);
void handle_hlt(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) {}

void op_add(State8080 *state, uint16_t value)
{
	uint16_t result = (uint16_t)state->general_register[A] + (uint16_t)value;
	set_flags(state, result);

	state->general_register[A] = (uint8_t)result;
}
RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(ADD, ADD_END_RANGE, ADD_IMMEDIATE); // ADC immediate value to A register
HANDLE_SINGLE_TARGET_OPCODES(ADD, op_add, ADD, ADD_IMMEDIATE);

void op_adc(State8080 *state, uint16_t value) { op_add(state, value + state->cc[CARRY]); }
RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(ADC, ADC_END_RANGE, ADC_IMMEDIATE); // ADC immediate value to A register
HANDLE_SINGLE_TARGET_OPCODES(ADC, op_adc, ADC, ADC_IMMEDIATE);

void op_sub(State8080 *state, uint16_t value)
{
	uint16_t result = (uint16_t)state->general_register[A] - value;
	state->cc[CARRY] = state->general_register[A] < value;
	state->general_register[A] = (uint8_t)result;
	set_flagsZSP(state, state->general_register[A]);
}

RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(SUB, SUB_END_RANGE, SUB_IMMEDIATE); // SUB immediate value from A register
HANDLE_SINGLE_TARGET_OPCODES(SUB, op_sub, SUB, SUB_IMMEDIATE);

void op_sbb(State8080 *state, uint16_t value) { op_sub(state, value + state->cc[CARRY]); }
RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(SBB, SBB_END_RANGE, SBB_IMMEDIATE); // SUB immediate value from A register
HANDLE_SINGLE_TARGET_OPCODES(SBB, op_sbb, SBB, SBB_IMMEDIATE);

void op_ana(State8080 *state, uint8_t value)
{
	state->general_register[A] = state->general_register[A] & value;
	set_flagsZSP(state, state->general_register[A]);
	state->cc[CARRY] = 0; // Clear carry flag
}

RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(ANA, ANA_END_RANGE, ANA_IMMEDIATE); // ANA immediate value with A register
HANDLE_SINGLE_TARGET_OPCODES(ANA, op_ana, ANA, ANA_IMMEDIATE);

void op_xra(State8080 *state, uint8_t value)
{
	state->general_register[A] ^= value;
	set_flagsZSP(state, state->general_register[A]);
	state->cc[CARRY] = 0; // Clear carry flag
}

RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(XRA, XRA_END_RANGE, XRA_IMMEDIATE); // XRA immediate value with A register
HANDLE_SINGLE_TARGET_OPCODES(XRA, op_xra, XRA, XRA_IMMEDIATE);

void op_ora(State8080 *state, uint8_t value)
{
	state->general_register[A] = state->general_register[A] | value;
	set_flagsZSP(state, state->general_register[A]);
	state->cc[CARRY] = 0; // Clear carry flag
}

RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(ORA, ORA_END_RANGE, ORA_IMMEDIATE); // ORA immediate value with A register
HANDLE_SINGLE_TARGET_OPCODES(ORA, op_ora, ORA, ORA_IMMEDIATE);

void op_cmp(State8080 *state, uint16_t value)
{
	uint8_t result = state->general_register[A] - value;
	set_flagsZSP(state, result);
	state->cc[CARRY] = state->general_register[A] < value; // Set carry flag if A < value
}

RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(CMP, CMP_END_RANGE, CMP_IMMEDIATE); // CMP immediate value with A register
HANDLE_SINGLE_TARGET_OPCODES(CMP, op_cmp, CMP, CMP_IMMEDIATE);

void op_lxi(State8080 *state, SPECIAL_REGISTER special_register, uint16_t value)
{
	if (is_pair(special_register))
	{
		set_register_pair(state, special_register, value);
	}
	else
	{
		state->sp = value;
	}
}

DISCRETE_VALUE_SHOULD_HANDLE(LXI, LXI_B, LXI_D, LXI_H, LXI_SP);
void handle_lxi(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint16_t value_to_set = bytes_to_word(opcode[1], opcode[2]);
	SPECIAL_REGISTER special_register_to_set = *opcode / 0x10; // opcodes layout distance is 0x10.
	op_lxi(state, special_register_to_set, value_to_set);
	state->pc += 2;
}

void op_stax(State8080 *state, SPECIAL_REGISTER special_register)
{
	uint16_t address = get_register_pair(state, special_register);
	write_to_memory(state, address, state->general_register[A]);
}
DISCRETE_VALUE_SHOULD_HANDLE(STAX, STAX_B, STAX_D);
void handle_stax(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER special_register = *opcode / 0x10; // opcodes layout distance is 0x10.
	op_stax(state, special_register);
}

void op_call(State8080 *state, uint8_t adr_low, uint8_t adr_high)
{
	uint16_t ret_address = state->pc + 2;
	uint8_t high = ((ret_address >> 8) & 0xff);
	uint8_t low = ret_address & 0xff;
	push(state, low, high);
	state->pc = bytes_to_word(adr_low, adr_high);
}

void op_ret(State8080 *state)
{
	uint8_t low = pop_byte(state);
	uint8_t high = pop_byte(state);
	state->pc = bytes_to_word(low, high);
}

void op_jmp(State8080 *state, uint8_t low, uint8_t high) { state->pc = bytes_to_word(low, high); }

static CONDITION_CODE condition_type_to_condition_code(CONDITION_TYPE condition_type)
{
	switch (condition_type)
	{
	case NZ:
	case Z:
		return ZERO;
	case NC:
	case CY:

		return CARRY;
	case PO:
	case PE:
		return PARITY;
	case PLUS:
	case MINUS:
		return SIGN;
	default:
		return 0; // TODO: Throw an error or assert here
	}
}

/// <summary>
/// Determines whether a conditional 8080 operation (such as CALL, RET, or JMP)
/// should be executed, based on the CPU condition flags and the instruction opcode.
///
/// Utilize the conditional opcodes (e.g., RC, RNC, RZ, RNZ, etc.) layout which have
/// a fixed, evenly spaced pattern relative to their base opcode (CALL, JMP, RET), and that each
/// condition appears in pairs: one inverted (e.g., RNZ) and one non-inverted (e.g., RZ).
///
/// The logic works as follows:
/// - Unconditional opcodes (CALL, JMP, RET) are always executed and return true immediately.
/// - The condition type is inferred from the distance between the current opcode and the base opcode.
/// - The condition is inverted if the type index is even (e.g., RNZ, RNC, RPO, RM).
/// - The relevant CPU flag is checked, and the result is inverted if needed.
///
/// This approach allows a generic handler for all conditional control flow operations,
/// avoiding hardcoded checks for each specific opcode.
/// </summary>
/// <param name="state">The current CPU state, including flags.</param>
/// <param name="opcode">The current instruction's opcode.</param>
/// <param name="base_opcode"> First condition opcode for each type of operation (CNZ, RNZ, JNZ).
/// The base opcode for the **unconditional** form of the instruction
/// (e.g., RET = 0xC9, JMP = 0xC3, CALL = 0xCD).
/// </param>
/// <returns>True if the condition is satisfied and the operation should proceed; otherwise false.</returns>
bool should_do_conditional_op(State8080 *state, OPCODE opcode, OPCODE base_opcode)
{
	if (opcode == JMP || opcode == CALL || opcode == RET)
	{ // No condition case
		return true;
	}

	CONDITION_TYPE condition_type = (opcode - base_opcode) / NUMBER_OF_CONDITIONS;
	bool invert_flag = (condition_type % 2) == 0;
	CONDITION_CODE condition_code = condition_type_to_condition_code(condition_type);

	bool condition_met = state->cc[condition_code];
	return invert_flag ? !condition_met : condition_met;
}

DISCRETE_VALUE_SHOULD_HANDLE(JMP, JMP, JZ, JNZ, JC, JNC, JPE, JPO, JM, JP);
void handle_jmp(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	if (should_do_conditional_op(state, *opcode, JNZ))
	{
		op_jmp(state, opcode[1], opcode[2]);
	}
	else
	{
		state->pc += 2;
	}
}

DISCRETE_VALUE_SHOULD_HANDLE(RET, RET, RZ, RNZ, RC, RNC, RPE, RPO, RM, RP);
void handle_ret(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	if (should_do_conditional_op(state, *opcode, RNZ))
	{
		op_ret(state);
	}
}

DISCRETE_VALUE_SHOULD_HANDLE(CALL, CALL, CZ, CNZ, CC, CNC, CPE, CPO, CM, CP);
void handle_call(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	if (should_do_conditional_op(state, *opcode, CNZ))
	{
		op_call(state, opcode[1], opcode[2]);
	}
	else
	{
		state->pc += 2;
	}
}

static void op_inr_memory(State8080 *state, SPECIAL_REGISTER register_pair)
{
	uint16_t address = get_register_pair(state, register_pair);
	uint8_t newValue = read_from_memory(state, address) + 1;
	write_to_memory(state, address, newValue);
	set_flagsZSP(state, newValue);
}

static void op_inr_byte(State8080 *state, uint8_t *value)
{
	(*value)++;
	set_flagsZSP(state, *value);
}

DISCRETE_VALUE_SHOULD_HANDLE(INR, INR_B, INR_C, INR_D, INR_E, INR_H, INR_L, INR_HL, INR_A); // Increment register or memory at HL address
void handle_op_inr(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	GENERAL_REGISTER register_to_increment = *opcode / 0x8; // opcodes layout distance is 0x8.
	if (register_to_increment == HL_GENERAL_REGISTER)
	{
		op_inr_memory(state, HL);
	}
	else
	{
		op_inr_byte(state, &state->general_register[register_to_increment]);
	}
}

static void op_inx_pair(State8080 *state, SPECIAL_REGISTER register_pair)
{
	uint16_t value = get_register_pair(state, register_pair);
	set_register_pair(state, register_pair, value + 1);
}

DISCRETE_VALUE_SHOULD_HANDLE(INX, INX_B, INX_D, INX_H, INX_SP); // Increment register or memory at HL address
void handle_op_inx(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER special_register_to_increment = *opcode / 0x10; // opcodes layout distance is 0x10.
	if (is_pair(special_register_to_increment))
	{
		op_inx_pair(state, special_register_to_increment);
	}
	else
	{
		state->sp++;
	}
}

SINGLE_VALUE_SHOULD_HANDLE(STA);
void handle_sta(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint16_t address = bytes_to_word(opcode[1], opcode[2]);
	write_to_memory(state, address, state->general_register[A]);
	state->pc += 2; // Move past the STA instruction
}

static void op_DCR_byte(State8080 *state, uint8_t *value)
{
	(*value)--;
	set_flagsZSP(state, *value);
}

static void op_DCR_memory(State8080 *state, SPECIAL_REGISTER register_pair)
{
	uint16_t address = get_register_pair(state, register_pair);
	uint8_t newValue = read_from_memory(state, address) - 1;
	write_to_memory(state, address, newValue);
	set_flagsZSP(state, newValue);
}

DISCRETE_VALUE_SHOULD_HANDLE(DCR, DCR_B, DCR_C, DCR_D, DCR_E, DCR_H, DCR_L, DCR_HL, DCR_A); // Decrement register or memory at HL address
void handle_dcr(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	GENERAL_REGISTER register_to_decrement = *opcode / 0x8; // opcodes layout distance is 0x8.
	if (register_to_decrement == HL_GENERAL_REGISTER)
	{
		op_DCR_memory(state, HL);
	}
	else
	{
		op_DCR_byte(state, &state->general_register[register_to_decrement]);
	}
}

static void op_daa(State8080 *state)
{
	uint8_t correction = 0;

	// Step 1: low nibble
	if ((state->general_register[A] & 0x0F) > 9)
	{
		correction += 0x06;
	}

	// Step 2: high nibble
	if (((state->general_register[A] >> 4) > 9) || state->cc[CARRY])
	{
		correction += 0x60;
	}

	// Now apply correction and check if carry occurs
	uint16_t result = (uint16_t)state->general_register[A] + correction;

	state->general_register[A] = (uint8_t)result;
	set_flags(state, result);
}

SINGLE_VALUE_SHOULD_HANDLE(DAA);
void handle_daa(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	op_daa(state);
}

static void op_in(Cpu8080 *cpu, State8080 *state, uint8_t port)
{
	uint8_t value = cpu->inTask.readPort(port, cpu->inTask.context);
	state->general_register[A] = value;
	state->pc++;
}

SINGLE_VALUE_SHOULD_HANDLE(IN);
void handle_in(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	op_in(cpu, state, opcode[1]);
}

static void op_out(Cpu8080 *cpu, State8080 *state, uint8_t port)
{
	cpu->outTask.writePort(port, state->general_register[A], cpu->outTask.context);
	state->pc++;
}
SINGLE_VALUE_SHOULD_HANDLE(OUT);
void handle_out(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	op_out(cpu, state, opcode[1]);
}

DISCRETE_VALUE_SHOULD_HANDLE(DI_EI, DI, EI); // Disable and Enable Interrupts
void handle_di_ei(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	state->interrupt_enable = *opcode == EI; // Disable interrupts
}

static void op_pchl(State8080 *state)
{
	state->pc = get_register_pair(state, HL);
}

SINGLE_VALUE_SHOULD_HANDLE(PCHL); // Load register pair immediate value
void handle_pchl(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	op_pchl(state);
}

static op_mvi(State8080 *state, GENERAL_REGISTER source_register, uint8_t target_value)
{
	if (source_register == HL_GENERAL_REGISTER)
	{
		write_to_register_pair_address(state, HL, target_value);
	}
	else
	{
		state->general_register[source_register] = target_value;
	}
	state->pc++;
}

DISCRETE_VALUE_SHOULD_HANDLE(MVI, MVI_B, MVI_C, MVI_D, MVI_E, MVI_H, MVI_L, MVI_M, MVI_A)
void handle_mvi(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	GENERAL_REGISTER source_register = *opcode / 0x8;
	uint8_t target_value = opcode[1];
	op_mvi(state, source_register, target_value);
}

static void op_dad(State8080 *state, uint32_t value)
{
	uint32_t hl = get_register_pair(state, HL);
	uint32_t combined = hl + value;
	set_register_pair(state, HL, combined & 0xffff);
	state->cc[CARRY] = (combined >> 16) & 1;
}

DISCRETE_VALUE_SHOULD_HANDLE(DAD, DAD_B, DAD_D, DAD_H, DAD_SP);
void handle_dad(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER register_to_add = *opcode / 0x10;
	uint32_t value_to_add = is_pair(register_to_add) ? get_register_pair(state, register_to_add) : state->sp;

	op_dad(state, value_to_add);
}

SINGLE_VALUE_SHOULD_HANDLE(STC); // Load register pair immediate value
void handle_stc(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	state->cc[CARRY] = true;
}

SINGLE_VALUE_SHOULD_HANDLE(CMC); // Load register pair immediate value
void handle_cmc(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	state->cc[CARRY] = !state->cc[CARRY];
}

static void op_dcx(State8080 *state, SPECIAL_REGISTER special_register)
{
	if (is_pair(special_register))
	{
		uint16_t value = get_register_pair(state, special_register);
		set_register_pair(state, special_register, value - 1);
	}
	else
	{
		state->sp--;
	}
}

DISCRETE_VALUE_SHOULD_HANDLE(DCX, DCX_B, DCX_D, DCX_H, DCX_SP); // Decrement register or memory at HL address
void handle_dcx(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER special_register = *opcode / 0x10; // opcodes layout distance is 0x10.
	op_dcx(state, special_register);
}

static void op_ldax(State8080 *state, SPECIAL_REGISTER register_pair)
{
	state->general_register[A] = read_from_register_pair_address(state, register_pair);
}

DISCRETE_VALUE_SHOULD_HANDLE(LDAX, LDAX_B, LDAX_D);
void handle_ldax(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER register_pair = *opcode / 0x10; // opcodes layout distance is 0x10.
	op_ldax(state, register_pair);
}

static void op_push_psw(State8080 *state)
{
	push(state, flags_to_byte(state), state->general_register[A]);
}

DISCRETE_VALUE_SHOULD_HANDLE(PUSH, PUSH_BC, PUSH_DE, PUSH_HL, PUSH_PSW);
void handle_push(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER special_register = (*opcode - PUSH_BC) / 0x10; // opcodes layout distance is 0x10.
	is_pair(special_register) ? push_pair(state, special_register) : op_push_psw(state);
}

static void op_pop_psw(State8080 *state)
{
	byte_to_flags(state, pop_byte(state));
	state->general_register[A] = pop_byte(state);
}
DISCRETE_VALUE_SHOULD_HANDLE(POP, POP_BC, POP_DE, POP_HL, POP_PSW);
void handle_pop(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	SPECIAL_REGISTER special_register = (*opcode - POP_BC) / 0x10; // opcodes layout distance is 0x10.
	is_pair(special_register) ? pop_to_pair(state, special_register) : op_pop_psw(state);
}

DISCRETE_VALUE_SHOULD_HANDLE(RST, RST0, RST1, RST2, RST3, RST4, RST5, RST6, RST7);
void handle_rst(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint8_t rst_number = (*opcode - RST0) / 8; // RST opcodes are spaced by 8
	op_rst(state, rst_number);
}

static void op_lda(State8080 *state, uint16_t address)
{
	state->general_register[A] = read_from_memory(state, address);
	state->pc += 2;
}

SINGLE_VALUE_SHOULD_HANDLE(LDA);
void handle_lda(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint16_t address = bytes_to_word(opcode[1], opcode[2]);
	op_lda(state, address);
}

static void op_rlc(State8080 *state)
{
	uint8_t prevBit7 = state->general_register[A] >> 7;
	state->general_register[A] = (state->general_register[A] << 1) & 0xfe; // Rotate left, clear low order bit
	state->general_register[A] |= state->cc[CARRY];						   // Set low order bit to carry
	state->cc[CARRY] = prevBit7;
}

static void op_rrc(State8080 *state)
{
	uint8_t prevBit0 = state->general_register[A] & 1;
	state->general_register[A] = (state->general_register[A] >> 1) | (prevBit0 << 7);
	state->cc[CARRY] = prevBit0;
}

static void op_ral(State8080 *state)
{
	uint8_t prevBit7 = state->general_register[A] >> 7;
	state->general_register[A] = (state->general_register[A] << 1) & 0x7f; // Rotate left, clear last bit
	state->general_register[A] |= (state->cc[CARRY] << 7);				   // Set last bit to carry
	state->cc[CARRY] = prevBit7;
}

static void op_rar(State8080 *state)
{
	uint8_t prevBit0 = state->general_register[A] & 1;

	state->general_register[A] = (state->general_register[A] >> 1) | (state->cc[CARRY] << 7);
	state->cc[CARRY] = prevBit0;
}

SINGLE_VALUE_SHOULD_HANDLE(RLC);
SINGLE_VALUE_SHOULD_HANDLE(RRC);
SINGLE_VALUE_SHOULD_HANDLE(RAL);
SINGLE_VALUE_SHOULD_HANDLE(RAR);
void handle_rlc(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_rlc(state); }
void handle_rrc(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_rrc(state); }
void handle_ral(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_ral(state); }
void handle_rar(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_rar(state); }

static void op_sphl(State8080 *state) { state->sp = get_register_pair(state, HL); }
SINGLE_VALUE_SHOULD_HANDLE(SPHL);
void handle_sphl(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_sphl(state); }

static void op_xchg(State8080 *state)
{
	uint16_t tmpHL = get_register_pair(state, HL);
	set_register_pair(state, HL, get_register_pair(state, DE));
	set_register_pair(state, DE, tmpHL);
}
SINGLE_VALUE_SHOULD_HANDLE(XCHG);
void handle_xchg(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_xchg(state); }

static void op_xthl(State8080 *state)
{
	uint8_t tmpL = state->general_register[L];
	uint8_t tmpH = state->general_register[H];

	state->general_register[L] = read_from_memory(state, state->sp);
	state->general_register[H] = read_from_memory(state, state->sp + 1);

	write_to_memory(state, state->sp, tmpL);
	write_to_memory(state, state->sp + 1, tmpH);
}
SINGLE_VALUE_SHOULD_HANDLE(XTHL);
void handle_xthl(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_xthl(state); }

static void op_cma(State8080 *state) { state->general_register[A] = ~state->general_register[A]; }
SINGLE_VALUE_SHOULD_HANDLE(CMA);
void handle_cma(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) { op_cma(state); }

static void op_shld(State8080 *state, uint16_t address)
{
	write_to_memory(state, address, state->general_register[L]);
	write_to_memory(state, address + 1, state->general_register[H]);
}
SINGLE_VALUE_SHOULD_HANDLE(SHLD);
void handle_shld(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint16_t address = bytes_to_word(opcode[1], opcode[2]);
	op_shld(state, address);
	state->pc += 2;
}

static op_lhld(State8080 *state, uint16_t address)
{
	state->general_register[L] = read_from_memory(state, address);
	state->general_register[H] = read_from_memory(state, address + 1);
}
SINGLE_VALUE_SHOULD_HANDLE(LHLD);
void handle_lhld(Cpu8080 *cpu, State8080 *state, uint8_t *opcode)
{
	uint16_t address = bytes_to_word(opcode[1], opcode[2]);
	op_lhld(state, address);
	state->pc += 2;
}

const OpcodeHandler handlers[] = {
	{is_value_part_of_LXI, handle_lxi},
	{is_value_SHLD, handle_shld},
	{is_value_LHLD, handle_lhld},
	{is_value_DAA, handle_daa},
	{is_value_CMA, handle_cma},
	{is_value_STA, handle_sta},
	{is_value_STC, handle_stc},
	{is_value_CMC, handle_cmc},
	{is_value_IN, handle_in},
	{is_value_OUT, handle_out},
	{is_value_part_of_DCR, handle_dcr},
	{is_value_part_of_DAD, handle_dad},
	{is_value_part_of_LDAX, handle_ldax},
	{is_value_part_of_DI_EI, handle_di_ei},
	{is_value_PCHL, handle_pchl},
	{is_value_SPHL, handle_sphl},
	{is_value_XTHL, handle_xthl},
	{is_value_XCHG, handle_xchg},

	{is_value_part_of_INR, handle_op_inr},
	{is_value_part_of_INX, handle_op_inx},

	{is_value_part_of_STAX, handle_stax},

	{is_between_MOV_MOV_END_RANGE_without_HLT, handle_mov},
	{is_value_part_of_MVI, handle_mvi},
	{is_value_part_of_DCX, handle_dcx},
	{is_value_LDA, handle_lda},
	{is_value_HLT, handle_hlt},
	{is_value_part_of_NOP, handle_nop},

	{is_value_RLC, handle_rlc},
	{is_value_RRC, handle_rrc},
	{is_value_RAL, handle_ral},
	{is_value_RAR, handle_rar},

	{is_between_ADD_ADD_END_RANGE_or_ADD_IMMEDIATE, handle_ADD},
	{is_between_ADC_ADC_END_RANGE_or_ADC_IMMEDIATE, handle_ADC},
	{is_between_SUB_SUB_END_RANGE_or_SUB_IMMEDIATE, handle_SUB},
	{is_between_SBB_SBB_END_RANGE_or_SBB_IMMEDIATE, handle_SBB},
	{is_between_ANA_ANA_END_RANGE_or_ANA_IMMEDIATE, handle_ANA},
	{is_between_XRA_XRA_END_RANGE_or_XRA_IMMEDIATE, handle_XRA},
	{is_between_ORA_ORA_END_RANGE_or_ORA_IMMEDIATE, handle_ORA},
	{is_between_CMP_CMP_END_RANGE_or_CMP_IMMEDIATE, handle_CMP},

	{is_value_part_of_POP, handle_pop},
	{is_value_part_of_PUSH, handle_push},

	{is_value_part_of_RST, handle_rst},
	// Conditional opcodes
	{is_value_part_of_CALL, handle_call},
	{is_value_part_of_RET, handle_ret},
	{is_value_part_of_JMP, handle_jmp},

};
const handlers_size = sizeof(handlers) / sizeof(OpcodeHandler);
