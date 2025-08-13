#pragma once
#include "SpaceInvaderMachine.h"


void poll_keys(KeyPress key_presses_buffer[MAX_KEY_PRESSES], MachineState* machine_state);

void sdl_handle_system_events(MachineState* machine_state);