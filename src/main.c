#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "SDL2/SDL.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef unsigned int uint;

#include "beta.h"
#include "term.h"
#include "cga.h"

#include "betalib/betalib.h"

const int MEMORY_SIZE = 4 * 1024;

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

lua_State* lstate;

SDL_Window* window = NULL;
SDL_Surface* screen = NULL;

Terminal* terminal;
CGA* adapter;

bool running = true;
bool busy = false;
bool interrupt = false;

void openlualibs(lua_State *l) {
	static const luaL_Reg lualibs[] = {
		{ "base", luaopen_base },
		{ "betalib", luaopen_betalib },
		{ NULL, NULL },
	};

	const luaL_Reg *lib;

	for (lib = lualibs; lib->func != NULL; lib++) {
		lib->func(l);
		lua_settop(l, 0);
	}

	luaL_openlibs(l);
}

void cleanup() {
	lua_close(lstate);
	SDL_Quit();
}

void sdl_init() {
	setbuf(stdout, NULL);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "could not initialize SDL2: %s\n", SDL_GetError());
		exit(1);
  }

  window = SDL_CreateWindow(
		"Gamma",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN
	);

  if (window == NULL) {
    fprintf(stderr, "could not create window: %s\n", SDL_GetError());
		exit(1);
  }

  screen = SDL_GetWindowSurface(window);
  SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0xFF, 0xFF, 0xFF));
  SDL_UpdateWindowSurface(window);
	
	atexit(cleanup);
}

void script_run(lua_State *L, const char* fn) {
	int s = luaL_loadfile(L, fn);

	if (s == 0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);

		if (s) {
			fprintf(stderr, "Lua error: %s\n", lua_tostring(L, -1));
			exit(0);
		}

		printf("initialized\n");
	} else {
		fprintf(stderr, "Lua load error: %s\n", lua_tostring(L, -1));
		exit(0);
	}
}

void *beta_run(void *ptr) {
	while (running && !beta->halted) {
		beta_tick(beta, lstate);

		if (interrupt) {
			beta_interrupt(beta, lstate, interrupt);
			interrupt = false;
		}

		usleep(100000);
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	// Check for correct # of arguments
	if (argc != 2) {
		printf("error: not enough arguments\n");
		printf("usage: gamma /path/to/script.lua\n");
		exit(1);
	}

	// Check that script file exists
	FILE* file = fopen(argv[1], "r");
	if (file == NULL) {
		printf("error: script does not exist\n");
		printf("usage: gamma /path/to/script.lua\n");
		exit(1);
	} else {
		fclose(file);
	}

	lstate = luaL_newstate();

	// create a terminal
	terminal = term_create(SCREEN_WIDTH / CHARACTER_WIDTH, SCREEN_HEIGHT / CHARACTER_HEIGHT, "res/font.png");
	term_puts(terminal, "Hello world");
	adapter = cga_create(320, 200, 2);

	// Draw test pattern to make it easier to recognize if the CGA adapter
	// is not working
	for (int i = 0; i < 16; i++) {
		for (int y = 0; y < 16; y++) {
			for (int x = 0; x < 16; x++) {
				cga_set(adapter, i * 16 + x, y, i);
			}
		}
	}

	sdl_init();

	beta = beta_create(MEMORY_SIZE);

	// Run the load script
	openlualibs(lstate);
	script_run(lstate, argv[1]);

	// Start the Beta emulation in its own thread
	pthread_t beta_thread;
	pthread_create(&beta_thread, NULL, beta_run, NULL);

	while (true) {
		// Graphics
		memcpy(adapter->pixels, beta->graph_mem, sizeof(uint32_t) * adapter->width * adapter->height / 8);
		cga_render(adapter, screen, 0, 0);

		term_render(terminal, screen, 0, 400);
		SDL_UpdateWindowSurface(window);

		SDL_Event event;

		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;
					pthread_join(beta_thread, NULL);
					exit(0);
					break;
				case SDL_KEYDOWN:
					switch (event.key.keysym.sym) {
						case SDLK_RIGHT:
							break;
						case SDLK_LEFT:
							break;
						default: {
							const char* keyname = SDL_GetKeyName(event.key.keysym.sym);

							if (strlen(keyname) == 1) {
								printf("key = %s\n", keyname);
								beta->key = keyname[0];
								beta_interrupt(beta, lstate, VEC_KBD);
							} else if (event.key.keysym.sym == SDLK_KP_SPACE) {
								beta->key = ' ';
								beta_interrupt(beta, lstate, VEC_KBD);
							}

							break;
						}
					}
					break;
			}
		}

		SDL_Delay(32);
	}

	return 0;
}
