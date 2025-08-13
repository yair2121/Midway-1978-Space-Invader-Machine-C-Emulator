#pragma once

#include <MWDisplay.h>

typedef struct SDL_CONTEXT SDL_CONTEXT;

typedef struct DisplayParams_SDL {
	SDL_Window* window;
	SDL_Renderer* renderer;

	SDL_Texture* framebuffer_texture; // The texture updated each frame
	uint32_t* pixel_buffer; // Software buffer for pixel data (RGBA32)
} DisplayParams_SDL;

#define WIDOW_STARTING_SCALE 5 // Starting window size, scaled in comparison to the Space Invader resolution

void render_frame_SDL(COLOR_FILTER frame[FRAME_HEIGHT][FRAME_WIDTH], SDL_CONTEXT* context);

void destroy_renderer_SDL(SDL_CONTEXT* context);
void init_renderer_SDL(SDL_CONTEXT* context);

extern const SDL_Color SDL_COLORS[NUMBER_OF_COLORS];


