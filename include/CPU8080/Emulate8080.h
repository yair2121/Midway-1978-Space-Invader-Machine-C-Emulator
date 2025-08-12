#pragma once

#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

typedef enum GENERAL_REGISTER
{
	B = 0,
	C,
	D,
	E,
	H,
	L,
	HL_GENERAL_REGISTER, // Duplicated because many opcodes works with both HL pair and the rest of general registers (MOV, ADD, etc.)
	A,
	NUMBER_OF_OPCODE_REGISTERS,
	NUMBER_OF_GENERAL_REGISTERS = NUMBER_OF_OPCODE_REGISTERS - 1, // HL_GENERAL_REGISTER is not a general register, but a special case for opcode handling.
} GENERAL_REGISTER;

typedef enum SPECIAL_REGISTER
{
	BC = 0,
	DE,
	HL,
	NUMBER_OF_REGISTER_PAIRS,
	// SP and PSW always appears after the pairs in the opcode layout
	SP = NUMBER_OF_REGISTER_PAIRS,
	PSW = NUMBER_OF_REGISTER_PAIRS,
	PC
} SPECIAL_REGISTER;

typedef enum CONDITION_CODE
{
	CARRY = 0,
	PAD1,
	PARITY,
	PAD2,
	AC,
	PAD3,
	ZERO,
	SIGN,
	NUMBER_OF_CONDITION_CODES,
} CONDITION_CODE;

typedef struct State8080
{
	uint8_t general_register[NUMBER_OF_OPCODE_REGISTERS]; // Leaving index 6 (HL) unused to reuse same enum for both general registers and opcode registers.
	uint16_t sp;
	uint16_t pc;
	uint8_t *memory;
	bool cc[NUMBER_OF_CONDITION_CODES];
	// struct ConditionCodes cc;
	bool interrupt_enable;
} State8080;

/// <summary>
/// Read port value from the current machine into the 8080 cpu.
/// </summary>
typedef uint8_t read_port_func(uint8_t port, void *context);

/// <summary>
/// Write a value to given port on 8080 cpu to current machine.
/// </summary>
typedef void write_port_func(uint8_t port, uint8_t value, void *context);

/// <summary>
/// Holds both the read_port function, and the machine context required for it to actually read the port.
/// </summary>
typedef struct
{
	read_port_func *read_port;
	void *context;
} InTask;

/// <summary>
/// Holds both the write_port function, and the machine context required for it to actually write to the port.
/// </summary>
typedef struct
{
	write_port_func *write_port;
	void *context;
} OutTask;

typedef struct Cpu8080
{
	State8080 *state;
	InTask in_task;
	OutTask out_task;
} Cpu8080;

uint8_t get_next_opcode(State8080 *state);

/// <summary>

/// </summary>
/// <param name="state"></param>
/// <returns></returns>
int emulate_8080_op(Cpu8080 *cpu);

/// <summary>
/// Return the real number of cycles that will take for the 8080 cpu to run given opcode.
/// </summary>
/// <param name="opcode"></param>
/// <returns></returns>
int opcode_to_cycles(uint8_t opcode);

void generate_interrupt(State8080 *state, int interrupt_num);
Cpu8080 *init_cpu_state(size_t buffer_size, uint8_t *code_buffer, size_t memory_size);

/// <summary>
/// Set the communication between the cpu and its machine through the IO ports.
/// </summary>
/// <param name="cpu"></param>
/// <param name="in_task"></param>
/// <param name="out_task"></param>
void set_in_out_ports(Cpu8080 *cpu, InTask in_task, OutTask out_task);

void free_cpu(Cpu8080 *state);

void write_to_register_pair_address(State8080 *state, SPECIAL_REGISTER register_pair, uint8_t value);
uint8_t read_from_register_pair_address(State8080 *state, SPECIAL_REGISTER register_pair);
void set_flagsZSP(State8080 *state, uint8_t value);
void set_flags(State8080 *state, uint16_t value);
uint16_t get_register_pair(State8080 *state, SPECIAL_REGISTER register_pair);
void set_register_pair(State8080 *state, SPECIAL_REGISTER register_pair, uint16_t value);
uint16_t bytes_to_word(uint16_t low, uint16_t high);
void write_to_memory(State8080 *state, uint16_t offset, uint8_t value);
uint8_t read_from_memory(State8080 *state, uint16_t offset);
uint8_t pop_byte(State8080 *state);
void push(State8080 *state, uint8_t low, uint8_t high);
bool parity(int value, int size);
void byte_to_flags(State8080 *state, uint8_t flags);
uint8_t flags_to_byte(State8080 *state);

void op_rst(State8080* state, uint8_t N);
/// <summary>/// /// </summary>/// <param name="special_register"></param>/// <returns>Whether the given special_register is BC/DE/HL</returns>
bool is_pair(SPECIAL_REGISTER special_register);


void run_CPU(Cpu8080* cpu, uint64_t real_time_to_run, uint64_t speedScaling);