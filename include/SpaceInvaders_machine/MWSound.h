#pragma once
#include <stdbool.h>

typedef struct MachineState MachineState;


typedef enum SOUND_EFFECT_INVADERS {
	UFO = 0,
	SHOT,
	FLASH, // Player die
	INVADER_DIE,
	NUMBER_OF_SOUND_EFFECTS_PORT_3,
	FLEET_MOVEMENT_1,
	FLEET_MOVEMENT_2,
	FLEET_MOVEMENT_3,
	FLEET_MOVEMENT_4,
	UFO_HIT,
	NUMBER_OF_SOUND_EFFECTS_PORT_5 = 5,
	NUMBER_OF_SOUND_EFFECTS = NUMBER_OF_SOUND_EFFECTS_PORT_3 + NUMBER_OF_SOUND_EFFECTS_PORT_5
} SOUND_EFFECT_INVADERS;

static const char* const SOUND_EFFECT_NAMES[] = {
    "ufo.wav",          // 0
    "shot.wav",         // 1
    "player_die.wav",   // 2
    "invader_die.wav",  // 3
    "fleet_1.wav",      // 4
    "fleet_2.wav",      // 5
    "fleet_3.wav",      // 6
    "fleet_4.wav",      // 7
    "ufo_hit.wav"       // 8
};

typedef void (*play_sound_effects) (bool sound_effects_to_play[NUMBER_OF_SOUND_EFFECTS], void* platform_context);

void play_frame_sound_effects(MachineState* machine_state);
