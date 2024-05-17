#pragma once
#include <inttypes.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct ConditionCodes {
	uint8_t    z : 1;
	uint8_t    s : 1;
	uint8_t    p : 1;
	uint8_t    cy : 1;
	uint8_t    ac : 1;
	uint8_t    pad : 3;
} ConditionCodes;

typedef struct State8080 {
	uint8_t a;
	uint8_t b;
	uint8_t c;
	uint8_t d;
	uint8_t e;
	uint8_t h;
	uint8_t l;
	uint16_t sp;
	uint16_t pc;
	uint8_t* memory;
	struct ConditionCodes cc;
	uint8_t     interrupt_enable;
} State8080;

/// <summary>
/// Read port value from the current machine into the 8080 cpu.
/// </summary>
typedef uint8_t(*readPortFunc)(uint8_t port);

/// <summary>
/// Write a value to given port on 8080 cp to current machine.
/// </summary>
typedef uint8_t(*writePortFunc)(uint8_t port, uint8_t value);


typedef struct Cpu8080 {
	State8080* state;
	readPortFunc readPort;
	writePortFunc writePort;
} Cpu8080;


/// <summary>

/// </summary>
/// <param name="state"></param>
/// <returns></returns>
int emulate_8080_op(Cpu8080* cpu);

Cpu8080* init_cpu(readPortFunc readPort, writePortFunc writePort, size_t bufferSize, uint8_t* codeBuffer);

void free_cpu(Cpu8080* state);