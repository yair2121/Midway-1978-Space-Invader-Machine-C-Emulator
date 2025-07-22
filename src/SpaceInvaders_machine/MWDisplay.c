#include "MWDisplay.h"



DisplayParams* init_display_params(int width, int height, SDL_Color pixelColor)
{
	DisplayParams* displayParams = (DisplayParams*)malloc(sizeof(DisplayParams));

	if (displayParams == NULL) {
		SDL_Quit();
		return -1;
	}

	displayParams->screenParams = (DisplayParams*)malloc(sizeof(ScreenParams));
	if (displayParams->screenParams == NULL)
	{
		SDL_Quit();
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("Space Invaders", width, height, NULL);
	if (window == NULL) {
		SDL_Log("SDL_CreateWindow errors: %s", SDL_GetError());
		return -1;
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
	if (renderer == NULL) {
		SDL_Log("SDL_CreateRenderer errors: %s", SDL_GetError());
		return -1;
	}
	SDL_SetRenderDrawColor(renderer, pixelColor.r, pixelColor.g, pixelColor.b, pixelColor.a);

	displayParams->screenParams->width = width;
	displayParams->screenParams->height = height;
	displayParams->screenParams->pixelColor = pixelColor;
	displayParams->renderer = renderer;
	displayParams->window = window;

	return displayParams;
}


void fillScreen(DisplayParams* displayParams, uint8_t* frameBuffer)
{
	for (int row = 0; row < 256; row++) {
		for (int col = 0; col < 28; col++) {
			uint8_t nextColumn = frameBuffer[row * 32 + col];
			for (int bit = 0; bit < 8; bit++) {
				uint8_t pixel = (nextColumn >> bit) & 0x1;
				//printf("Painting x: %d, y: %d\n", (y * 8) + bit, 255 - x);

				if (pixel != 0) {
					if (SDL_RenderPoint(displayParams->renderer, row, 224 - ((col * 8) + bit)) == false) {
						SDL_Log("SDL_CreateRenderer errors: %s", SDL_GetError());
						return -1;
					}
				}
			}

		}
	}
}
void clearRenderer(DisplayParams* displayParams) {
	SDL_SetRenderDrawColor(displayParams->renderer, 0, 0, 0, 255);
	SDL_RenderClear(displayParams->renderer);
	SDL_SetRenderDrawColor(displayParams->renderer, displayParams->screenParams->pixelColor.r,
		displayParams->screenParams->pixelColor.g,
		displayParams->screenParams->pixelColor.b,
		displayParams->screenParams->pixelColor.a);
}
void reRenderScreen(DisplayParams* displayParams)
{
	SDL_RenderPresent(displayParams->renderer);
	//SDL_SetRenderDrawColor(displayParams->renderer, 0, 0, 0, 255);
	//SDL_RenderClear(displayParams->renderer);
	/*SDL_SetRenderDrawColor(displayParams->renderer, displayParams->screenParams->pixelColor.r,
		displayParams->screenParams->pixelColor.g,
		displayParams->screenParams->pixelColor.b,
		displayParams->screenParams->pixelColor.a);*/
}

void free_display_params(DisplayParams* displayParams)
{
	SDL_DestroyRenderer(displayParams->renderer);
	SDL_DestroyWindow(displayParams->window);
	free(displayParams->screenParams);
	free(displayParams);
	SDL_Quit();
}

void getFrame(uint8_t* frameBufferFromMemory, uint8_t** resultBuffer) {
	for (int row = 0; row < 256; row++) {
		for (int col = 0; col < 28; col++) {
			uint8_t nextColumn = frameBufferFromMemory[row * 32 + col];
			for (int bit = 0; bit < 8; bit++) {
				uint8_t pixel = (nextColumn >> bit) & 0x1;
				int current_col = (col * 8) + bit;
				resultBuffer[row][current_col] = pixel;
			}

		}
	}
}


static COLOR_FILTER pixel_to_color_filter(uint8_t x, uint8_t y) {

}
void applyColorFilter(uint8_t** frame, COLOR_FILTER** resultColoredFrame)
{
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y++) {
			if (frame[x][y] == 0) {
				resultColoredFrame[x][y] = BLACK;
			}
			else {
				resultColoredFrame[x][y] = pixel_to_color_filter(x, y);
			}
		}
	}
}
