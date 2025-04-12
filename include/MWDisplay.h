#pragma once
#include <SDL3/SDL.h>


typedef struct ScreenParams {
	int width;
	int height;
	SDL_Color pixelColor;
} ScreenParams;

typedef struct DisplayParams {
	ScreenParams* screenParams;
	SDL_Renderer* renderer;
	SDL_Window* window;
} DisplayParams;


DisplayParams* init_display_params(int width, int height, SDL_Color pixelColor);

/// <summary>
/// Draws the screen using the given buffer.
/// </summary>
/// <param name="displayParams"></param>
/// <param name="buffer">Video memory start point (which is mapped to 0x2400)</param>
void fillScreen(DisplayParams* displayParams, uint8_t* buffer);

void reRenderScreen(DisplayParams* displayParams);

void free_display_params(DisplayParams* displayParams);