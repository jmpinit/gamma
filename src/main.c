#include <stdio.h>
#include "SDL2/SDL.h"

SDL_Surface* screen;

void cleanup() {
	SDL_FreeSurface(screen);
}

int main(int argc, char* argv[]) {
	SDL_Window *window;                    // Declare a pointer

	SDL_Init(SDL_INIT_VIDEO);              // Initialize SDL2

	// Create an application window with the following settings:
	window = SDL_CreateWindow("An SDL2 window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);

	// Check that the window was successfully made
	if (window == NULL) {
		// In the event that the window could not be made...
		printf("Could not create window: %s\n", SDL_GetError());
		return 1;
	}

	SDL_Renderer *ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
	if (ren == NULL){
		printf("SDL_CreateRenderer Error: %s\n",SDL_GetError());
		return 1;
	}

	screen = SDL_CreateRGBSurface(0, 640, 480, 4, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
	if(screen == NULL) {
		fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, screen);
	SDL_FreeSurface(screen);
	if (tex == NULL){
		fprintf(stderr, "SDL_CreateTextureFromSurface failed: %s\n", SDL_GetError());
		exit(1);
		return 1;
	}

	SDL_RenderClear(ren);
	SDL_RenderCopy(ren, tex, NULL, NULL);
	SDL_RenderPresent(ren);

	// The window is open: enter program loop (see SDL_PollEvent)
	SDL_Delay(3000);  // Pause execution for 3000 milliseconds, for example

	SDL_DestroyTexture(tex);
	SDL_DestroyRenderer(ren);

	// Close and destroy the window
	SDL_DestroyWindow(window);

	// Clean up
	SDL_Quit();
	return 0;
}
