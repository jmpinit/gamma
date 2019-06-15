#define LUA_LIB

#include <stdio.h>
#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>

#include "betalib/betalib.h"

#include "betalib/debug.h"

static const luaL_Reg betalib[] = {
  // Initilializers
  { "new", betalib_new },

  // debug
  { "write_reg", betalib_write_reg },
  { "read_reg", betalib_read_reg },
  { "write_mem", betalib_write_mem },
  { "read_mem", betalib_read_mem },
  { "tick", betalib_tick },
  { "interrupt", betalib_interrupt },
  { "load", betalib_load },
  { NULL, NULL },
};

LUALIB_API int luaopen_betalib(lua_State *L) {
  luaL_register(L, "betalib", betalib);
  return 1;
}
