#pragma once

#include "Emulate8080.h"
#include "EmulateMWSpaceInvaders.h"

typedef struct MachineState {
	Cpu8080* cpu;
	MWState* mwState;
} MachineState;

MachineState* init_machine(size_t bufferSize, uint8_t* codeBuffer, size_t memorySize);

void free_machine(MachineState* machineState);


void run_CPU(MachineState* machineState, uint64_t lastRunTime, uint64_t currentRunTime);

void run_machine(MachineState* machineState);

