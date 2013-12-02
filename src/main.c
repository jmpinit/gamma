#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "SDL/SDL.h"
//#include "SDL/SDL_thread.h"

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "beta.h"
#include "term.h"

#include "betalib/betalib.h"

lua_State* lstate;
SDL_Surface* screen;

Terminal* terminal;

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
	terminal = term_init(0, 0, 640/11, 480/13/2, "res/font.png");

	SDL_init();
	SDL_EnableUNICODE(1);

	// load the libraries
	openlualibs(lstate);

	// run the load script
	script_run(lstate, argv[1]);

	beta = beta_create(4096);
	beta_load(beta, "checker.bin");

	while(true) {
		beta_tick(beta, lstate);
		if(beta->halted) exit(0);

		term_render(terminal, screen);
		SDL_Flip(screen);

		SDL_Event event;

		while(SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
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
