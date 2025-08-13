#pragma once

#include <SDL3/SDL.h>
#include "MWInput.h"
#include "SpaceInvaderMachine.h"

void poll_keys(KeyPress key_presses_buffer[MAX_KEY_PRESSES], MachineState* machine_state);

void sdl_handle_system_events(MachineState* machine_state);