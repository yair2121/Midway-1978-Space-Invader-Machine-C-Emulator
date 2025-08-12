#include "MWSound.h"
#include "SpaceInvaderMachine.h"

#include <stdbool.h>
#include <EmulateMWSpaceInvaders.h>
static void fill_sound_effects(bool sound_effects[NUMBER_OF_SOUND_EFFECTS], PortsState portsState) {
	for (SOUND_EFFECT_INVADERS sound_effect = UFO; sound_effect < NUMBER_OF_SOUND_EFFECTS; sound_effect++) {
		uint8_t* port;
		int sound_effect_bit = sound_effect;
		if (sound_effect < NUMBER_OF_SOUND_EFFECTS_PORT_3)
		{
			port = &portsState.sound_bits_1;
		}
		else {
			port = &portsState.sound_bits_2;
			sound_effect_bit -= NUMBER_OF_SOUND_EFFECTS_PORT_3; // Adjust index for port 5 sound effects
		}
		sound_effects[sound_effect] = (bool)((*port >> sound_effect_bit) & 1);
	}
}

/// <summary>
/// Play sound effect only if it started in this frame.
/// </summary>
/// <param name="previous_frame_state">Whether the sound effect was active during the previous frame</param>
/// <param name="current_frame_state">Whether the sound effect is active during the current frame</param>
/// <returns></returns>
static bool should_play_sound_effect(bool previous_frame_state, const bool current_frame_state) {
	return current_frame_state && !previous_frame_state;
}

/// <summary>
///  UFO sound effects persists across frames, while other sound effects are triggered only in the first frame they started.
/// </summary>
/// <param name="effects_to_play"></param>
/// <param name="current_effects"></param>
/// <param name="last_effects"></param>
static void determine_effects_to_play(bool effects_to_play[NUMBER_OF_SOUND_EFFECTS], const bool current_effects[NUMBER_OF_SOUND_EFFECTS], const bool last_effects[NUMBER_OF_SOUND_EFFECTS]) {
	effects_to_play[UFO] = current_effects[UFO]; // UFO sound is persistent - play whenever it's active

	for (SOUND_EFFECT_INVADERS effect = SHOT; effect < NUMBER_OF_SOUND_EFFECTS; effect++) {
		effects_to_play[effect] = should_play_sound_effect(last_effects[effect], current_effects[effect]);
	}
}
void play_frame_sound_effects(MachineState* machine_state) {
	bool previous_active_effects[NUMBER_OF_SOUND_EFFECTS];
	memcpy(previous_active_effects, machine_state->previous_active_sound_effects, sizeof(machine_state->previous_active_sound_effects));

	fill_sound_effects(&machine_state->previous_active_sound_effects, machine_state->mwState->ports);
	bool* current_sound_effects = machine_state->previous_active_sound_effects;

	bool sound_effects_to_play[NUMBER_OF_SOUND_EFFECTS] = { false }; // We fill only the ones that were added in this frame
	determine_effects_to_play(sound_effects_to_play, current_sound_effects, previous_active_effects);

	machine_state->play_sound_effects_func(sound_effects_to_play, machine_state->platform_context);
}