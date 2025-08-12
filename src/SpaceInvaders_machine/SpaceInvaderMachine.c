#include "SpaceInvaderMachine.h"

MachineState* init_machine(GAME_ROM game_rom, size_t memory_size, void* platform_context, play_sound_effects play_sound_effects_func, InputFunctions input_functions, DisplayFunctions display_functions, TimeFunctions time_functions) {
	MachineState* machine_state = (MachineState*)malloc(sizeof(MachineState));
	if (machine_state == NULL) {
		return NULL;
	}
	machine_state->cpu = init_cpu_state(game_rom.size, game_rom.code_buffer, memory_size);
	machine_state->cpu->state->sp = SP_START;

	machine_state->mwState = init_mw_state(machine_state->cpu);
	machine_state->platform_context = platform_context;

	machine_state->play_sound_effects_func = play_sound_effects_func;
	machine_state->display_functions = display_functions;
	machine_state->input_functions = input_functions;
	machine_state->time_functions = time_functions;

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
	machine_state->display_functions.free_renderer_func(machine_state->platform_context);
	free(machine_state);
}

static bool isValidKeyPress(KeyPress keyPress) {
	return keyPress.key != INVALID_KEY_PRESS.key && keyPress.player != INVALID_KEY_PRESS.player && keyPress.type != INVALID_KEY_PRESS.type;
}

static void handle_input(MachineState* machine_state) {
	KeyPress key_presses[MAX_KEY_PRESSES];
	for (int i = 0; i < MAX_KEY_PRESSES; i++) {
		key_presses[i] = INVALID_KEY_PRESS;
	}

	machine_state->input_functions.poll_key_presses_func(key_presses, machine_state);
	for (int i = 0; isValidKeyPress(key_presses[i]); i++) {
		machine_key_press(key_presses[i], &machine_state->mwState->ports);
	}
}

static void handle_render_frame(MachineState* machine_state) {
	static COLOR_FILTER frameBufferFromMemory[FRAME_HEIGHT][FRAME_WIDTH] = { 0 };
	get_colored_frame(get_frame_buffer(machine_state->cpu), frameBufferFromMemory);
	machine_state->display_functions.render_frame_func(frameBufferFromMemory, machine_state->platform_context);
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
	uint64_t last_run_time = machine_state->time_functions.microsecond_tick_func();
	uint64_t next_interrupt = last_run_time + 16666;
	int current_interrupt = 1;

	while (!machine_state->should_exit) {
		handle_input(machine_state);
		if (machine_state->is_running) {
			uint64_t currentRunTime = machine_state->time_functions.microsecond_tick_func();
			if (currentRunTime > next_interrupt && machine_state->cpu->state->interrupt_enable) {
				generate_interrupt(machine_state->cpu->state, current_interrupt + 1);
				//generate_interrupt(machine_state->cpu->state, 1);
				current_interrupt = (current_interrupt + 1) % 2;
				next_interrupt = currentRunTime + 8333;
				handle_render_frame(machine_state);
			}

			run_CPU(machine_state->cpu, currentRunTime - last_run_time, 1);
			play_frame_sound_effects(machine_state);
			last_run_time = currentRunTime;
		}
		machine_state->time_functions.sleep_func(1);
	}
}
