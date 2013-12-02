#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SDL/SDL.h"
#include "SDL/SDL_thread.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

typedef unsigned int uint;

#include "beta.h"
#include "term.h"
#include "cga.h"

#include "betalib/betalib.h"

lua_State* lstate;
SDL_Surface* screen;

Terminal* terminal;
CGA* adapter;

bool running = true;

void openlualibs(lua_State *l) {
	static const luaL_reg lualibs[] =
	{
		{ "base",       luaopen_base },
		{ "betalib",	luaopen_betalib },
		{ NULL,         NULL }
	};

	const luaL_reg *lib;

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
		printf("Could not initialize SDL: %s\n", SDL_GetError());
		exit(1);
	}
	
	screen = SDL_SetVideoMode(640, 480, 0, SDL_HWPALETTE);
	
	if (screen == NULL) {
		printf("Couldn't set screen mode to 640 x 480: %s\n", SDL_GetError());
		exit(1);
	}
	
	SDL_WM_SetCaption("gamma", NULL);
	
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

int BetaThread(void *ptr) {
	while(running) {
		beta_tick(beta, lstate);
	}

	return 0;
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

	lstate = lua_open();

	// create a terminal
	terminal = term_create(640/11, 240/13, "res/font.png");
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
	SDL_EnableUNICODE(1);

	// load the libraries
	openlualibs(lstate);

	beta = beta_create(4*1024);
	beta_load(beta, "src/asm/graphics.bin");

	// run the load script
	script_run(lstate, argv[1]);

	SDL_Thread *thread;
	thread = SDL_CreateThread(BetaThread, "BetaThread");

	if(NULL == thread) {
		printf("SDL_CreateThread failed: %s\n", SDL_GetError());
		exit(1);
	}

	while(true) {
		if(beta->halted) exit(0);

		// graphics
		memcpy(adapter->pixels, beta->graph_mem, sizeof(uint32_t)*adapter->width*adapter->height/8);
		cga_render(adapter, screen, 0, 0);

		term_render(terminal, screen, 0, 400);
		SDL_Flip(screen);

		SDL_Event event;

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					running = false;

					int threadReturnValue;
					SDL_WaitThread(thread, &threadReturnValue);

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
							//beta_interrupt(beta, VEC_KBD);
							break;
					}
					break;
			}
		}

		SDL_Delay(32);
	}

	return 0;
}
