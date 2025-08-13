#include "MWDisplay.h"


const Color_Screen_Boundaries COLOR_REGIONS[NUM_COLOR_REGIONS] = {
	/* Top section - WHITE (score display area) */
	{ WHITE, {0, 0}, {32, FRAME_WIDTH - 1} },

	/* Upper section - RED (invader formation area) */
	{ RED, {32, 0}, {64, FRAME_WIDTH - 1} },

	/* Upper middle section - WHITE (main gameplay area) */
	{ WHITE, {64, 0}, {183, FRAME_WIDTH - 1} },

	/* Space ship section - GREEN */
	{ GREEN, {184, 0}, {241, FRAME_WIDTH - 1} },


	/* Lower section - WHITE (left part of bottom area) */
	{ WHITE, {241, 0}, {FRAME_HEIGHT - 1, 25} },

	/* Lower section - GREEN (middle part of bottom area) */
	{ GREEN, {241, 25}, {FRAME_HEIGHT - 1, 136} },

	/* Lower section - WHITE (right part of bottom area) */
	{ WHITE, {241, 136}, {FRAME_HEIGHT - 1, FRAME_WIDTH - 1} }
};



static bool is_in_color_region(Color_Screen_Boundaries boundary, uint8_t x, uint8_t y) {
		return x >= boundary.start_left.x && x <= boundary.start_right.x &&
			y >= boundary.start_left.y && y <= boundary.start_right.y;
}

/// <summary>
/// Rotate the memory frame buffer 90 degrees counter clockwise to fit the original arcade machine display aspect ratio.
/// </summary>
/// <param name="frame_buffer_from_memory"></param>
/// <param name="result_buffer"></param>
void get_frame(uint8_t frame_buffer_from_memory[FRAME_BUFFER_SIZE], uint8_t result_buffer[FRAME_HEIGHT][FRAME_WIDTH]) {
	int buffer_width = FRAME_HEIGHT / 8; // Each byte in the frame buffer represents 8 pixels vertically
	int buffer_height = FRAME_WIDTH;

	for (int x = 0; x < buffer_height; x++) {
		for (int y = 0; y < buffer_width; y++) {
			uint8_t next_column = frame_buffer_from_memory[x * buffer_width + y];
			for (int bit = 0; bit < 8; bit++) {
				uint8_t pixel = (next_column >> bit) & 0x1;

				int final_row = FRAME_HEIGHT - (y * 8 + bit);    // Y coordinate (0-255)  
				int final_col = x; // X coordinate (0-223)
				result_buffer[final_row][final_col] = pixel;
			}
		}
	}
}

static COLOR_FILTER pixel_to_color_filter(uint8_t x, uint8_t y) {
	for (size_t i = 0; i < NUM_COLOR_REGIONS; i++)
	{
		if (is_in_color_region(COLOR_REGIONS[i], x, y)) {
			return COLOR_REGIONS[i].color;
		}
	}
}

void apply_color_filter(uint8_t frame[FRAME_HEIGHT][FRAME_WIDTH], COLOR_FILTER result_colored_frame[FRAME_HEIGHT][FRAME_WIDTH])
{
	for (int x = 0; x < FRAME_HEIGHT; x++) {
		for (int y = 0; y < FRAME_WIDTH; y++) {
			uint8_t pixel = frame[x][y];
			result_colored_frame[x][y] = pixel != 0 ? pixel_to_color_filter(x, y) : BLACK;
		}
	}
}

void get_colored_frame(uint8_t frame_buffer_from_memory[FRAME_BUFFER_SIZE], COLOR_FILTER result_buffer[FRAME_HEIGHT][FRAME_WIDTH]) {
	static uint8_t frame_pixels[FRAME_HEIGHT][FRAME_WIDTH];
	get_frame(frame_buffer_from_memory, frame_pixels);
	apply_color_filter(frame_pixels, result_buffer);
}


