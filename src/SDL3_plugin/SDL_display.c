#include "SDL_display.h"
#include "SDL_platform.h"


const SDL_Color SDL_COLORS[NUMBER_OF_COLORS] = {
	{ 0, 0, 0, 255 },     /* BLACK */
	{ 255, 255, 255, 255 }, /* WHITE */
	{ 255, 0, 0, 255 },   /* RED */
	{ 0, 255, 0, 255 }    /* GREEN */
};

// Convert COLOR_FILTER enum to 32-bit RGBA value
static uint32_t color_to_rgba(COLOR_FILTER color) {
    SDL_Color c = SDL_COLORS[color];
    return (uint32_t)c.a << 24 | (uint32_t)c.b << 16 | (uint32_t)c.g << 8 | (uint32_t)c.r; // Pack RGBA into 32-bit value for SDL_PIXELFORMAT_RGBA32

}

bool init_renderer_sdl(SDL_CONTEXT* context) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		SDL_Log("SDL_Init errors\n");
		return false;
	}
	DisplayParams_SDL* display_params = &context->display;

	SDL_PropertiesID props = SDL_CreateProperties();
	if (props == 0) {
		SDL_Log("Unable to create properties\n");
		return false;
	}

	SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Space Invaders");
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_RESIZABLE_BOOLEAN, true);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, FRAME_WIDTH * WIDOW_STARTING_SCALE);
	SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, FRAME_HEIGHT * WIDOW_STARTING_SCALE);
	
	display_params->window = SDL_CreateWindowWithProperties(props);
	if (!display_params->window) {
		SDL_Log("SDL_CreateWindow errors\n");
		return false;
	}

	display_params->renderer = SDL_CreateRenderer(display_params->window, NULL);
	if (!display_params->renderer) {
		SDL_Log("SDL_CreateRenderer failed\n");
		SDL_DestroyWindow(display_params->window);
		return false;
	}
	// Allocate pixel buffer
	display_params->pixel_buffer = (uint32_t*)malloc(FRAME_WIDTH * FRAME_HEIGHT * sizeof(uint32_t));
	if (!display_params->pixel_buffer) {
		SDL_Log("Failed to allocate pixel buffer memory\n");
		SDL_DestroyRenderer(display_params->renderer);
		SDL_DestroyWindow(display_params->window);
		return false;
	}

	// Create texture
	display_params->framebuffer_texture = SDL_CreateTexture(
		display_params->renderer,
		SDL_PIXELFORMAT_RGBA32,
		SDL_TEXTUREACCESS_STREAMING,
		FRAME_WIDTH,
		FRAME_HEIGHT
	);
	if (!display_params->framebuffer_texture) {
		SDL_Log("SDL_CreateTexture failed\n");
		free(display_params->pixel_buffer);
		SDL_DestroyRenderer(display_params->renderer);
		SDL_DestroyWindow(display_params->window);
		return false;
	}

	SDL_SetTextureScaleMode(display_params->framebuffer_texture, SDL_SCALEMODE_NEAREST);
	return true;
}


/**
 * @brief Computes a centered destination rectangle (SDL_FRect) that preserves a given aspect ratio
 *        within the current SDL window size, applying letterboxing or pillarboxing as needed.
 *
 * This function:
 *   1. Retrieves the current window pixel size via SDL_GetWindowSizeInPixels().
 *   2. Compares the window's aspect ratio to the target ratio.
 *   3. Computes the largest possible rectangle fitting inside the window while maintaining
 *      the fixed aspect ratio.
 *   4. Centers that rectangle horizontally or vertically, creating black borders if necessary.
 *
 * @param window        Pointer to an existing SDL_Window.
 * @param aspect_ratio  The desired width/height aspect ratio (e.g. 224.0f / 256.0f).
 * @return              SDL_FRect describing the destination rectangle
 *                      (x, y, w, h) into which rendering should occur.
 */
static SDL_FRect calculate_dst_rect(SDL_Window* window, float aspect_ratio) {
	int window_width, window_height;
	SDL_GetWindowSizeInPixels(window, &window_width, &window_height);

	float current_aspect = (float)window_width / (float)window_height;
	SDL_FRect dst;

	if (current_aspect > aspect_ratio) {
		// Window too wide (pillarbox)
		dst.h = (float)window_height;
		dst.w = dst.h * aspect_ratio;
		dst.x = ((float)window_width - dst.w) * 0.5f;
		dst.y = 0.0f;
	}
	else {
		// Window too tall or narrow (letterbox)
		dst.w = (float)window_width;
		dst.h = dst.w / aspect_ratio;
		dst.x = 0.0f;
		dst.y = ((float)window_height - dst.h) * 0.5f;
	}

	return dst;
}


/// <summary>
/// Convert a frame of COLOR_FILTER pixels to a pixel buffer in RGBA format.
/// </summary>
/// <param name="frame"></param>
/// <param name="pixel_buffer"></param>
static void frame_to_rgba(COLOR_FILTER frame[FRAME_HEIGHT][FRAME_WIDTH], uint32_t* pixel_buffer) {
	for (int y = 0; y < FRAME_HEIGHT; y++) {
		for (int x = 0; x < FRAME_WIDTH; x++) {
			pixel_buffer[y * FRAME_WIDTH + x] = color_to_rgba(frame[y][x]);
		}
	}
}

static clear_renderer(SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, SDL_COLORS[BLACK].r, SDL_COLORS[BLACK].g , SDL_COLORS[BLACK].b, SDL_COLORS[BLACK].a);
	SDL_RenderClear(renderer);

}
void render_frame_SDL(COLOR_FILTER frame[FRAME_HEIGHT][FRAME_WIDTH], SDL_CONTEXT* context) {
	DisplayParams_SDL* display_params_sdl = &context->display;
	frame_to_rgba(frame, display_params_sdl->pixel_buffer);

	// Upload pixel buffer to texture
	SDL_UpdateTexture(
		display_params_sdl->framebuffer_texture,
		NULL,
		display_params_sdl->pixel_buffer,
		FRAME_WIDTH * sizeof(uint32_t)
	);

	clear_renderer(display_params_sdl->renderer);

	int window_width, window_height;
	SDL_GetWindowSize(display_params_sdl->window, &window_width, &window_height);

	float current_window_aspect_ratio = (float)window_width / (float)window_height;
	SDL_FRect dst = calculate_dst_rect(display_params_sdl->window, SPACE_INVADERS_ASPECT_RATIO);

	SDL_RenderTexture(display_params_sdl->renderer, display_params_sdl->framebuffer_texture, NULL, &dst);
	SDL_RenderPresent(display_params_sdl->renderer);
}

void destroy_renderer_sdl(DisplayParams_SDL* display_params_sdl) {
	SDL_DestroyTexture(display_params_sdl->framebuffer_texture);
	free(display_params_sdl->pixel_buffer);

	SDL_DestroyRenderer(display_params_sdl->renderer);
	SDL_DestroyWindow(display_params_sdl->window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


