#include "SDL_platform.h"


void sdl_play_sound_effects(bool sound_effects[NUMBER_OF_SOUND_EFFECTS], SDL_CONTEXT* platform_context) {
	for (SOUND_EFFECT_INVADERS sound_effect_invader = 0; sound_effect_invader < NUMBER_OF_SOUND_EFFECTS; sound_effect_invader++) {
		SoundEffectParams_SDL sound_effect_params = platform_context->sound_effects.params[sound_effect_invader];

		if (sound_effects[sound_effect_invader] & !is_playing_sound_effect(sound_effect_params)) {
			play_sound_effect(sound_effect_params);
		}

	}
}