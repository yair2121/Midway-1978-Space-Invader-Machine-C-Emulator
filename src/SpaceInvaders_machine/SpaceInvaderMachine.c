#include "SpaceInvaderMachine.h"


static void open_game_rom(GameRom* game_rom, char rom_path[PATH_BUFFER_SIZE]) {
	FILE* code_fp = fopen(rom_path, "rb");
	if (code_fp == NULL) {
		printf("Not able to open the file space invader rom. Please add it to the %s.", rom_path);
		return;
	}

	fseek(code_fp, 0L, SEEK_END);
	size_t fileSize = ftell(code_fp);
	fseek(code_fp, 0L, SEEK_SET);

	game_rom->code_buffer = (uint8_t*)malloc(fileSize);
	if (game_rom->code_buffer == NULL) {
		printf("Memory allocation failed.");
		return;
	}
	game_rom->size = fread(game_rom->code_buffer, 1, fileSize, code_fp); // TODO: Validate that return value equal fileSize
	fclose(code_fp);
}

MachineState* init_machine(char rom_path[PATH_BUFFER_SIZE], PlatformInterface platform_interface) {
	GameRom game_rom;
	open_game_rom(&game_rom, rom_path);
	if (game_rom.size == 0 || game_rom.code_buffer == NULL) {
		printf("Failed to load game ROM from %s", rom_path);
		return NULL;
	}

	MachineState* machine_state = (MachineState*)malloc(sizeof(MachineState));
	if (machine_state == NULL) {
		printf("Failed to allocate memory for the MachineState");
		return NULL;
	}

	machine_state->cpu = init_cpu_state(game_rom.size, game_rom.code_buffer, INVADERS_RAM_SIZE);
	if (machine_state->cpu == NULL) {
		free(game_rom.code_buffer);
		free(machine_state);
		return NULL;
	}
	machine_state->cpu->state->sp = SP_START;

	machine_state->mwState = init_mw_state(machine_state->cpu);
	if (machine_state->mwState == NULL) {
		free_cpu(machine_state->cpu);
		free(game_rom.code_buffer);
		free(machine_state);
		return NULL;
	}

	machine_state->platform_interface = platform_interface;

	for (SOUND_EFFECT_INVADERS sound_effect = UFO; sound_effect < NUMBER_OF_SOUND_EFFECTS; sound_effect++) {
		machine_state->previous_active_sound_effects[sound_effect] = false; // Initialize all sound effects to false.
	}	

	machine_state->is_running = false;
	machine_state->should_exit = false;
	return machine_state;

}

void free_machine(MachineState* machine_state) {
	free_cpu(machine_state->cpu);
	free_MWState(machine_state->mwState);
	free(machine_state);
}

static void handle_input(MachineState* machine_state) {
	KeyPress key_presses[MAX_KEY_PRESSES];
	for (int i = 0; i < MAX_KEY_PRESSES; i++) {
		key_presses[i] = INVALID_KEY_PRESS;
	}
	machine_state->platform_interface.events.poll_key_presses_func(key_presses, machine_state);
	for (int i = 0; is_valid_key_press(key_presses[i]); i++) {
		machine_key_press(key_presses[i], &machine_state->mwState->ports);
	}
}

static void handle_render_frame(MachineState* machine_state) {
	static COLOR_FILTER frameBufferFromMemory[FRAME_HEIGHT][FRAME_WIDTH] = { 0 };
	get_colored_frame(get_frame_buffer(machine_state->cpu), frameBufferFromMemory);
	machine_state->platform_interface.display.render_frame_func(frameBufferFromMemory, machine_state->platform_interface.platform_context);
}

bool toggle_machine_running(MachineState* machine_state) {
	machine_state->is_running = !machine_state->is_running;
	return machine_state->is_running;
}

void exit_machine(MachineState* machine_state) {
	machine_state->is_running = false;
	machine_state->should_exit = true;
}

void run_machine(MachineState* machine_state) {
	uint64_t last_run_time = machine_state->platform_interface.time.microsecond_tick_func();
	uint64_t next_interrupt = last_run_time + MILLISECOND_PER_FRAME;
	int current_interrupt = 1;

	while (!machine_state->should_exit) {
		handle_input(machine_state);
		machine_state->platform_interface.events.poll_system_events_func(machine_state);
		if (machine_state->is_running) {
			uint64_t currentRunTime = machine_state->platform_interface.time.microsecond_tick_func();
			if (currentRunTime > next_interrupt && machine_state->cpu->state->interrupt_enable) {
				generate_interrupt(machine_state->cpu->state, current_interrupt + 1);
				current_interrupt = (current_interrupt + 1) % 2;
				next_interrupt = currentRunTime + MILLISECOND_PER_FRAME / 2; // Half frame
				handle_render_frame(machine_state);
			}

			run_CPU(machine_state->cpu, currentRunTime - last_run_time, 1);
			play_frame_sound_effects(machine_state);
			last_run_time = currentRunTime;
		}
		machine_state->platform_interface.time.sleep_func(1);
	}
}
