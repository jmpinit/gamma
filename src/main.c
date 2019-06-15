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
	static const luaL_Reg lualibs[] =
	{
		{ "base",       luaopen_base },
		{ "betalib",	luaopen_betalib },
		{ NULL,         NULL }
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

void SDL_init() {
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

	if (s==0) {
		s = lua_pcall(L, 0, LUA_MULTRET, 0);
		if(s) {
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
	while(running && !beta->halted) {
		beta_tick(beta, lstate);
		if(interrupt) {
			beta_interrupt(beta, lstate, interrupt);
			interrupt = false;
		}

		usleep(100000);
	}

	return NULL;
}

int main(int argc, char* argv[]) {
	// chcek for correct # of arguments
	if(argc != 2) {
		printf("error: not enough arguments\n");
		printf("usage: gamma /path/to/script.lua\n");
		exit(1);
	}

	// check that script file exists
	FILE* file = fopen(argv[1], "r");
	if(file == NULL) {
		printf("error: script does not exist\n");
		printf("usage: gamma /path/to/script.lua\n");
		exit(1);
	} else {
		fclose(file);
	}

	lstate = luaL_newstate();

	// create a terminal
	terminal = term_create(SCREEN_WIDTH/11, SCREEN_HEIGHT/13, "res/font.png");
	adapter = cga_create(320, 200, 2);

	// draw test pattern FIXME
	for(int i=0; i < 16; i++) {
		for(int y=0; y < 16; y++) {
			for(int x=0; x < 16; x++) {
				cga_set(adapter, i*16 + x, y, i);
			}
		}
	}

	SDL_init();

	// load the libraries
	openlualibs(lstate);

	beta = beta_create(4*1024);

	// for testing manually set a simple program
	beta->memory[0] = 0;
	beta_load(beta, "graphics.bin");

	// run the load script
	script_run(lstate, argv[1]);

	pthread_t beta_thread;
	pthread_create(&beta_thread, NULL, beta_run, NULL);

	while(true) {
		//if(beta->halted) exit(0);

		// graphics
		memcpy(adapter->pixels, beta->graph_mem, sizeof(uint32_t)*adapter->width*adapter->height/8);
		cga_render(adapter, screen, 0, 0);

		//term_render(terminal, screen, 0, 400);
		SDL_UpdateWindowSurface(window);

		SDL_Event event;

		while(SDL_PollEvent(&event)) {
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
						default:
							//beta->key = event.key.keysym.unicode;
							//beta_interrupt(beta, lstate, VEC_KBD);
							break;
					}
					break;
			}
		}

		SDL_Delay(32);
	}

	return 0;
}
