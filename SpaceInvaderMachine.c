#include "SpaceInvaderMachine.h"
#include <SDL3/SDL.h>

MachineState* init_machine(size_t bufferSize, uint8_t* codeBuffer, size_t memorySize) {
	MachineState* machineState = (MachineState*)malloc(sizeof(MachineState));
	if (machineState == NULL) {
		return NULL;
	}
	machineState->cpu = init_cpu_state(bufferSize, codeBuffer, memorySize);
	machineState->mwState = init_mw_state(machineState->cpu);
	return machineState;
}

void free_machine(MachineState* machineState) {
	free_cpu(machineState->cpu);
	free(machineState->mwState);
	free(machineState);
}

/// <summary>
/// 
/// </summary>
/// <param name="machineState"></param>
/// <param name="lastRunTime">The last time we ran the machine, in microseconds</param>
/// <param name="currentRunTime">The program current run time, in microseconds</param>
void run_CPU(MachineState* machineState, uint64_t lastRunTime, uint64_t currentRunTime) {
	uint64_t cyclesToRun = (currentRunTime - lastRunTime) * 2; // 2Mhz
	for (int cyclesRan = 0; cyclesRan < cyclesToRun;)
	{
		uint8_t opcode = get_next_opcode(machineState->cpu->state);
		cyclesRan += opcode_to_cycles(opcode);
		emulate_8080_op(machineState->cpu);

	}
}


void run_machine(MachineState* machineState) {
	//uint64_t lastRunTime = get_microseconds_since_start();

	////while (true)
	////{
	//	uint64_t currentRunTime = get_microseconds_since_start();
	//	
	//	//if (!machineState->cpu->state->interrupt_enable) return;
	//	float secondsSinceLastInterupt = ((clock() - lastInterupt) / CLOCKS_PER_SEC);
	//	if (secondsSinceLastInterupt > 1.0 / 60.0) {

	//		GenerateInterrupt(cpu->state, 2);
	//		lastInterupt = clock();
	//	}
	//	runCPU(machineState, lastRunTime, currentRunTime);
	//	lastRunTime = currentRunTime;
	//}
}