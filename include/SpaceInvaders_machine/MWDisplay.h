#pragma once
#include <SDL3/SDL.h>
#include "Emulate8080.h"
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


typedef enum COLOR_FILTER
{
	WHITE = 0,
	BLACK,
	RED,
	GREEN,
} COLOR_FILTER;


DisplayParams* init_display_params(int width, int height, SDL_Color pixelColor);

/// <summary>
/// Draws the screen using the given buffer.
/// </summary>
/// <param name="displayParams"></param>
/// <param name="buffer">Video memory start point (which is mapped to 0x2400)</param>
void fillScreen(DisplayParams* displayParams, uint8_t* buffer);

void clearRenderer(DisplayParams* displayParams);
void reRenderScreen(DisplayParams* displayParams);
void free_display_params(DisplayParams* displayParams);


void getFrame(uint8_t* frameBufferFromMemory, uint8_t** resultBuffer);
void applyColorFilter(uint8_t** frame, COLOR_FILTER** resultColoredFrame);
