#ifndef	DEBUG_H 
#define DEBUG_H

int betalib_new(lua_State *L);

int betalib_write_reg(lua_State *L);
int betalib_read_reg(lua_State *L);

int betalib_write_mem(lua_State *L);
int betalib_read_mem(lua_State *L);

int betalib_tick(lua_State *L);
int betalib_interrupt(lua_State *L);

int betalib_load(lua_State *L);

// int betalib_pc(lua_State *L); TODO

#endif
