#include <stdio.h>
#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "betalib/betalib.h"

#include "betalib/debug.h"

// LIBRARY INITIALIZATION

static const luaL_reg betalib[] = {
	// initilializers
	{ "new",		betalib_new },

	// debug
	{ "write_reg",	betalib_write_reg },
	{ "read_reg",	betalib_read_reg },
	{ "write_mem",	betalib_write_mem },
	{ "read_mem",	betalib_read_mem },
	{ "tick",		betalib_tick },
	{ "interrupt",	betalib_interrupt },
	{ "load",		betalib_load },

	{ NULL,							NULL }
};

int luaopen_betalib(lua_State *L) {
	luaL_openlib(L, "betalib", betalib, 0);
	return 1;
}
