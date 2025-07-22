#include "Emulate8080.h"
#include <stdio.h>
#include<stdlib.h>
#include <string.h>
#include <time.h>
int main(int argc, char* argv[]) {
    FILE* codeFp;
	int err = fopen_s(&codeFp, argv[1], "rb");
	if (err != NULL) {
		printf("Not able to open the file.");
		exit(1); // What to return here?
	}

	fseek(codeFp, 0L, SEEK_END);
	size_t fileSize = ftell(codeFp);
	fseek(codeFp, 0L, SEEK_SET);

	unsigned char* buffer = (unsigned char*)malloc(fileSize);
	if (buffer == NULL) {
		printf("Memory allocation failed.");
		exit(1);
	}
	size_t size = fread(buffer, 1, fileSize, codeFp); // TODO: Validate that return value equal fileSize
	fclose(codeFp);


	Cpu8080* cpu = init_cpu_state(size, buffer, 0x10000);
    
    //set_in_out_ports()

    //unsigned int addr = 0;
    //unsigned int files = argc - 1;
    //if (strcmp(argv[1], "--start") == 0) {
    //    sscanf(argv[2], "%x", &addr);
    //    argv += 2;
    //    files -= 2;

    //    emulator.pc = addr;
    //}

    //read_rom_into_memory(&emulator, addr, ++argv, files);

    // When testing the CPU we want to skip the DAA test.  This instruction
    // is not needed for Space invaders to run properly.  We also need to fix
    // the stack pointer.
#ifdef CPU_TEST
    // Fix the stack pointer from 0x6ad to 0x7ad    
    // this 0x06 byte 112 in the code, which is    
    // byte 112 + 0x100 = 368 in memory 
    cpu->state->memory[368] = 0x7;

    // Skip DAA test    
    cpu->state->memory[0x59d] = 0xc3; //JMP
    cpu->state->memory[0x59e] = 0xc2;
    cpu->state->memory[0x59f] = 0x05;
#endif

    printf("Starting test!\n");
    while (1) {
        emulate_8080_op(cpu);
    }
    return 0;
}