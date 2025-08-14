#pragma once

#include <SDL3/SDL.h>
#include "Emulate8080.h"

#define FRAME_WIDTH 224
#define FRAME_HEIGHT 256
#define SPACE_INVADERS_ASPECT_RATIO ((float)FRAME_WIDTH / (float)FRAME_HEIGHT)
#define FRAME_BUFFER_SIZE ((FRAME_WIDTH * FRAME_HEIGHT) / 8) // Each byte in the frame buffer represents 8 pixels vertically
#define NUM_COLOR_REGIONS 7

typedef enum COLOR_FILTER {
	BLACK = 0,
	WHITE,
	RED,
	GREEN,
	NUMBER_OF_COLORS,
} COLOR_FILTER;

typedef struct ScreenPosition {
	uint8_t x; // X coordinate (0-255)
	uint8_t y; // Y coordinate (0-223)
} ScreenPosition;

typedef struct Color_Screen_Boundaries {
	COLOR_FILTER color; // Color of the screen area
	ScreenPosition start_left; // X coordinate (0-255)
	ScreenPosition start_right; // Y coordinate (0-223)
} Color_Screen_Boundaries;

/// <summary>
/// Screen boundaries for each color region.
/// </summary>
extern const Color_Screen_Boundaries COLOR_REGIONS[NUM_COLOR_REGIONS];

void get_frame(uint8_t frameBufferFromMemory[FRAME_BUFFER_SIZE], uint8_t resultBuffer[FRAME_HEIGHT][FRAME_WIDTH]);

/// <summary>
/// Convert binary frame (1s pixel on screen, 0s pixel off) to colored frame.
/// </summary>
/// <param name="frame"></param>
/// <param name="resultColoredFrame"></param>
void apply_color_filter(uint8_t frame[FRAME_HEIGHT][FRAME_WIDTH], COLOR_FILTER resultColoredFrame[FRAME_HEIGHT][FRAME_WIDTH]);


/// <summary>
/// Convert Game frame buffer to colored frame buffer (Based on Space Invader Arcade machine filter).
/// </summary>
/// <param name="frameBufferFromMemory"></param>
/// <param name="resultBuffer"></param>
void get_colored_frame(uint8_t frameBufferFromMemory[FRAME_BUFFER_SIZE], COLOR_FILTER resultBuffer[FRAME_HEIGHT][FRAME_WIDTH]);



