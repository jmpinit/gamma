#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "beta.h"

#include "betalib/debug.h"

#define ARGS "(memsize)"
int betalib_new(lua_State *L) {
  if(lua_type(L, 1) == LUA_TNUMBER) {
    // Get args
    int memsize = (int)lua_tonumber(L, 1);

    // Construct the Beta
    Beta* newbeta = beta_create(memsize);

    // Update state
    beta = newbeta;
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 0;
}
#undef ARGS

#define ARGS "(value, index)"
int betalib_write_reg(lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER &&
    lua_type(L, 2) == LUA_TNUMBER
  ) {
    // Get args
    int value = (int)lua_tonumber(L, 1);
    int index = (int)lua_tonumber(L, 2);

    beta_write_reg(beta, value, index);
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 0;
}
#undef ARGS

#define ARGS "(index)"
int betalib_read_reg(lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
    // Get args
    int index = (int)lua_tonumber(L, 1);

    lua_pushnumber(L, beta_read_reg(beta, index));
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 1;
}
#undef ARGS

#define ARGS "(value, address)"
int betalib_write_mem(lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER && lua_type(L, 2) == LUA_TNUMBER) {
    // Get args
    int value = (int)lua_tonumber(L, 1);
    int address = (int)lua_tonumber(L, 2);

    beta_write_mem(beta, value, address);
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 0;
}
#undef ARGS

#define ARGS "(address)"
int betalib_read_mem(lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
    // Get args
    int address = (int)lua_tonumber(L, 1);

    lua_pushnumber(L, beta_read_mem(beta, address));
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 1;
}
#undef ARGS

int betalib_tick(lua_State *L) {
  beta_tick(beta, L);
  return 0;
}

#define ARGS "(address)"
int betalib_interrupt(lua_State *L) {
  if (lua_type(L, 1) == LUA_TNUMBER) {
    // Get args
    int address = (int)lua_tonumber(L, 1);

    beta_interrupt(beta, L, address);
  } else {
    return luaL_error(L, "%s: argument should be %s", __func__, ARGS);
  }

  return 0;
}
#undef ARGS

// (filename)
int betalib_load(lua_State *L) {
  if (lua_type(L, 1) == LUA_TSTRING) {
    // Get args
    const char* filename = lua_tostring(L, 1);

    beta_load(beta, filename);

    return 1;
  } else {
    return luaL_error(L, "%s: argument should be filename.", __func__);
  }
}
