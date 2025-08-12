#pragma once


#include <inttypes.h>
#include <stdbool.h>

#include "Emulate8080.h"


typedef bool (*should_handle_func) (uint8_t opcode);
typedef void (*handle_opcode) (Cpu8080* cpu, State8080* state, uint8_t* opcode);
typedef struct {
	const should_handle_func should_handle_func;
	const handle_opcode handle_func;
} OpcodeHandler;


typedef enum OPCODE {
	SHLD = 0x22,
	LHLD = 0x2a,
	DAA = 0x27,
	CMA = 0x2f,
	STA = 0x32,
	STC = 0x37,
	CMC = 0x3f,
	HLT = 0x76,
	OUT = 0xd3,
	IN = 0xdb,
	PCHL = 0xe9,
	DI = 0xf3,
	EI = 0xfb,
	SPHL = 0xf9,
	XTHL = 0xe3,
	XCHG = 0xeb,
	LDA = 0x3a,

	LXI_B = 0x01, 
	LXI_D = 0x11,
	LXI_H = 0x21,
	LXI_SP = 0x31,

	STAX_B = 0x02,
	STAX_D = 0x12,

	INX_B = 0x03,
	INX_D = 0x13,
	INX_H = 0x23,
	INX_SP = 0x33,

	INR_B = 0x04,
	INR_C = 0x0c,
	INR_D = 0x14,
	INR_E = 0x1c,
	INR_H = 0x24,
	INR_L = 0x2c,
	INR_HL = 0x34,
	INR_A = 0x3c,

	DCR_B = 0x05,
	DCR_C = 0x0d,
	DCR_D = 0x15,
	DCR_E = 0x1d,
	DCR_H = 0x25,
	DCR_L = 0x2d,
	DCR_HL = 0x35,
	DCR_A = 0x3d,

	DAD_B = 0x09,
	DAD_D = 0x19,
	DAD_H = 0x29,
	DAD_SP = 0x39,

	LDAX_B = 0x0a,
	LDAX_D = 0x1a,

	MVI_B = 0x06,
	MVI_C = 0x0e,
	MVI_D = 0x16,
	MVI_E = 0x1e,
	MVI_H = 0x26,
	MVI_L = 0x2e,
	MVI_M = 0x36,
	MVI_A = 0x3e,

	RLC = 0x07,
	RRC = 0x0f,
	RAL = 0x17,
	RAR = 0x1f,

	DCX_B = 0x0b,
	DCX_D = 0x1b,
	DCX_H = 0x2b,
	DCX_SP = 0x3b,


	MOV = 0x40,
	MOV_END_RANGE = MOV + NUMBER_OF_OPCODE_REGISTERS * NUMBER_OF_OPCODE_REGISTERS - 1,

	ADD = 0x80,
	ADD_END_RANGE = ADD + NUMBER_OF_OPCODE_REGISTERS - 1,
	ADD_IMMEDIATE = 0xc6, // ADD immediate value to A register
	ADC = 0x88,
	ADC_END_RANGE = ADC + NUMBER_OF_OPCODE_REGISTERS - 1,
	ADC_IMMEDIATE = 0xce, // ADC immediate value to A register

	SUB = 0x90,
	SUB_END_RANGE = SUB + NUMBER_OF_OPCODE_REGISTERS - 1,
	SUB_IMMEDIATE = 0xd6, // SUB immediate value from A register
	SBB = 0x98,
	SBB_END_RANGE = SBB + NUMBER_OF_OPCODE_REGISTERS - 1,
	SBB_IMMEDIATE = 0xde, // SBB immediate value from A register

	ANA = 0xa0,
	ANA_END_RANGE = ANA + NUMBER_OF_OPCODE_REGISTERS - 1,
	ANA_IMMEDIATE = 0xe6, // ANA immediate value with A register

	XRA = 0xa8,
	XRA_END_RANGE = XRA + NUMBER_OF_OPCODE_REGISTERS - 1,
	XRA_IMMEDIATE = 0xee, // XRA immediate value with A register

	ORA = 0xb0,
	ORA_END_RANGE = ORA + NUMBER_OF_OPCODE_REGISTERS - 1,
	ORA_IMMEDIATE = 0xf6, // ORA immediate value with A register

	CMP = 0xb8,
	CMP_END_RANGE = CMP + NUMBER_OF_OPCODE_REGISTERS - 1,
	CMP_IMMEDIATE = 0xfe, // CMP immediate value with A register

	POP_BC = 0xc1,
	POP_DE = 0xd1,
	POP_HL = 0xe1,
	POP_PSW = 0xf1,


	PUSH_BC = 0xc5,
	PUSH_DE = 0xd5,
	PUSH_HL = 0xe5,
	PUSH_PSW = 0xf5,

	// CALL instructions
	CALL = 0xcd,
	CNZ = 0xc4,
	CZ = 0xcc,
	CNC = 0xd4,
	CC = 0xdc,
	CPO = 0xe4,
	CPE = 0xec,
	CP = 0xf4,
	CM = 0xfc,
	
	// RST
	RST0 = 0xc7,
	RST1 = 0xcf,
	RST2 = 0xd7,
	RST3 = 0xdf,
	RST4 = 0xe7,
	RST5 = 0xef,
	RST6 = 0xf7,
	RST7 = 0xff,


	// RET instructions
	RET = 0xc9,
	RNZ = 0xc0,
	RZ = 0xc8,
	RNC = 0xd0,
	RC = 0xd8,
	RPO = 0xe0,
	RPE = 0xe8,
	RP = 0xf0,
	RM = 0xf8,

	// JMP instructions
	JMP = 0xc3,
	JNZ = 0xc2,
	JZ = 0xca,
	JNC = 0xd2,
	JC = 0xda,
	JPO = 0xe2,
	JPE = 0xea,
	JP = 0xf2,
	JM = 0xfa,
} OPCODE;

typedef enum CONDITION_TYPE {
	NZ = 0,
	Z,
	NC,
	CY,
	PO,
	PE,
	PLUS,
	MINUS,
	NUMBER_OF_CONDITIONS,
	NO_CONDITION,
} CONDITION_TYPE;

extern const OpcodeHandler handlers[];
extern const int handlers_size;

#define SINGLE_VALUE_SHOULD_HANDLE(VALUE) bool is_value_##VALUE(uint8_t opcode) { return opcode == VALUE;}
#define RANGE_VALUE_SHOULD_HANDLE(MIN, MAX) bool is_between_##MIN##_##MAX(uint8_t opcode) { return opcode >= MIN && opcode <= MAX;}
#define RANGE_AND_SINGLE_VALUE_SHOULD_HANDLE(MIN, MAX, VALUE) bool is_between_##MIN##_##MAX##_or_##VALUE(uint8_t opcode) { return (opcode >= MIN && opcode <= MAX) || opcode == VALUE;}
#define RANGE_WITH_HOLE_VALUE_SHOULD_HANDLE(MIN, MAX, HOLE) bool is_between_##MIN##_##MAX##_without_##HOLE(uint8_t opcode) { return opcode >= MIN && opcode <= MAX && opcode != HOLE;}
#define DISCRETE_VALUE_SHOULD_HANDLE(NAME, ...)                          \
    static bool is_value_part_of_##NAME(uint8_t opcode) {                              \
        const uint8_t valid_opcodes[] = { __VA_ARGS__ };                 \
        for (size_t i = 0; i < sizeof(valid_opcodes) / sizeof(valid_opcodes[0]); ++i) { \
            if (opcode == valid_opcodes[i]) return true;                \
        }                                                                \
        return false;                                                    \
    }

/// <summary>
/// This is the handler format for ADD, ADC, SUB, SBB, ANA, XRA, ORA, CMP opcodes.
/// </summary>
#define HANDLE_SINGLE_TARGET_OPCODES(OP_NAME, OP_METHOD, OP_START, OP_IMMIDIATE)                          \
    void handle_##OP_NAME(Cpu8080 *cpu, State8080 *state, uint8_t *opcode) {                              \
        uint16_t value_to_##OP_NAME = get_single_target_value(state, opcode, OP_START, OP_IMMIDIATE);                 \
		if(*opcode == OP_IMMIDIATE) state->pc++;																			\
        OP_METHOD(state, value_to_##OP_NAME);              \
    }

#define DECLARE_SINGLE_TARGET_HANDLER(OP_NAME) \
    void handle_##OP_NAME(Cpu8080* cpu, State8080* state, uint8_t* opcode);



void handle_mov(Cpu8080* cpu, State8080* state, uint8_t* opcode);
DECLARE_SINGLE_TARGET_HANDLER(ADD);
DECLARE_SINGLE_TARGET_HANDLER(ADC);
DECLARE_SINGLE_TARGET_HANDLER(SUB_IMMEDIATE);	
DECLARE_SINGLE_TARGET_HANDLER(SUB);
DECLARE_SINGLE_TARGET_HANDLER(SBB);
DECLARE_SINGLE_TARGET_HANDLER(ANA);
DECLARE_SINGLE_TARGET_HANDLER(XRA);



DECLARE_SINGLE_TARGET_HANDLER(ORA);
DECLARE_SINGLE_TARGET_HANDLER(CMP);



void handle_cmp(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_lxi(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_stax(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_jmp(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_ret(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_call(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_op_inr(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_op_inx(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_sta(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_dcr(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_daa(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_di_ei(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_pchl(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_mvi(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_dad(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_stc(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_cmc(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_dcx(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_ldax(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_push(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_pop(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_rst(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_lda(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_rlc(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_rrc(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_ral(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_rar(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_sphl(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_xchg(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_xthl(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_cma(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_shld(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_lhld(Cpu8080* cpu, State8080* state, uint8_t* opcode);


void handle_in(Cpu8080* cpu, State8080* state, uint8_t* opcode);
void handle_out(Cpu8080* cpu, State8080* state, uint8_t* opcode);
