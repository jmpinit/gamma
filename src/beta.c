#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "beta.h"

Beta* beta;

Beta* beta_create(int progsize) {
  Beta* newbeta = calloc(1, sizeof(Beta));

  newbeta->tty = terminal;
  newbeta->screen = adapter;

  newbeta->memsize = progsize + // Main memory
    adapter->width*adapter->height / sizeof(uint32_t); // Mapped graphics data
  newbeta->memory = calloc(newbeta->memsize, sizeof(uint32_t));

  newbeta->graph_mem = newbeta->memory + progsize;

  return newbeta;
}

void beta_load(Beta* beta, const char* filename) {
  FILE *fp = fopen(filename, "rb");
  int size_loaded = fread(beta->memory, sizeof(uint32_t), beta->memsize, fp);

  if(size_loaded > beta->memsize) {
    printf("beta_load: expected %d u32s but got %d (too much).\n", beta->memsize, size_loaded);
    exit(1);
  } else {
    printf("loaded %d u32s into beta.\n", size_loaded);
  }
}

uint32_t beta_read_reg(Beta* beta, uint8_t index) {
  index &= 0x1F; // Register indices are 5 bits

  if(index == 31) {
    return 0;
  } else {
    return beta->registers[index];
  }
}

void beta_write_reg(Beta* beta, uint32_t value, uint8_t index) {
  index &= 0x1F; // Register indices are 5 bits

  if (index != 31) {
    beta->registers[index] = value;
  }
}

uint32_t beta_read_mem(Beta* beta, uint32_t addr) {
  addr >>= 2;

  if (addr < beta->memsize) {
    return beta->memory[addr];
  } else {
    printf("read: %x out of bounds\n", addr);
  }

  return 0;
}

void beta_write_mem(Beta* beta, uint32_t value, uint32_t addr) {
  addr >>= 2;

  if (addr < beta->memsize) {
    beta->memory[addr] = value;
  } else {
    printf("write: %x out of bounds\n", addr);
  }
}

void beta_interrupt(Beta* beta, lua_State *L, uint32_t address) {
  beta_write_reg(beta, beta->pc + 4, REG_XP);
  beta->pc = address | (1 << 31);
}

void beta_tick(Beta* beta, lua_State *L) {
  if (beta->halted) {
    return;
  }

  static uint32_t last_pc = 0;

  if (beta->pc != last_pc) {
    // PC updated somewhere else. Likely by an interrupt
    last_pc = beta->pc;
  }

  // Get next instruction
  uint32_t instruction = beta_read_mem(beta, beta->pc & ~(1 << 31));

  // Decode instruction fields
  uint8_t opcode = instruction >> 26;
  uint8_t reg_c = (instruction >> (26-5)) & 0x1F;
  uint8_t reg_a = (instruction >> (26-10)) & 0x1F;
  uint8_t reg_b = (instruction >> (26-15)) & 0x1F;
  int16_t literal = instruction & 0xFFFF;

  // Get data
  uint32_t val_c = beta_read_reg(beta, reg_c);
  uint32_t val_a = beta_read_reg(beta, reg_a);
  uint32_t val_b = beta_read_reg(beta, reg_b);

  // Execute instruction
  uint32_t res, ea;
  char c;
  switch(opcode) {
    case ADD:
      beta_write_reg(beta, val_a + val_b, reg_c);
      beta->pc += 4;
      break;
    case ADDC:
      beta_write_reg(beta, val_a + literal, reg_c);
      beta->pc += 4;
      break;
    case AND:
      beta_write_reg(beta, val_a & val_b, reg_c);
      beta->pc += 4;
      break;
    case ANDC:
      beta_write_reg(beta, val_a & literal, reg_c);
      beta->pc += 4;
      break;
    case BEQ:
      beta->pc += 4;
      ea = beta->pc + ((int32_t)literal) * 4;
      beta_write_reg(beta, beta->pc, reg_c);
      if (val_a == 0) {
        beta->pc = ea;
      }
      break;
    case BNE:
      beta->pc += 4;
      ea = beta->pc + ((int32_t)literal) * 4;
      beta_write_reg(beta, beta->pc, reg_c);
      if (val_a != 0) {
        beta->pc = ea;
      }
      break;
    case CMPEQ:
      beta->pc += 4;
      beta_write_reg(beta, (val_a == val_b)? 1: 0, reg_c);
      break;
    case CMPEQC:
      beta->pc += 4;
      beta_write_reg(beta, ((int32_t)val_a == (int32_t)literal)? 1: 0, reg_c);
      break;
    case CMPLE:
      beta->pc += 4;
      beta_write_reg(beta, ((int32_t)val_a <= (int32_t)val_b)? 1: 0, reg_c);
      break;
    case CMPLEC:
      beta->pc += 4;
      beta_write_reg(beta, ((int32_t)val_a <= (int32_t)literal)? 1: 0, reg_c);
      break;
    case CMPLT:
      beta->pc += 4;
      beta_write_reg(beta, ((int32_t)val_a < (int32_t)val_b)? 1: 0, reg_c);
      break;
    case CMPLTC:
      beta->pc += 4;
      beta_write_reg(beta, ((int32_t)val_a < (int32_t)literal)? 1: 0, reg_c);
      break;
    case DIV:
      beta_write_reg(beta, val_a / val_b, reg_c);
      beta->pc += 4;
      break;
    case DIVC:
      beta_write_reg(beta, val_a / literal, reg_c);
      beta->pc += 4;
      break;
    case JMP:
      beta->pc += 4;
      beta_write_reg(beta, beta->pc, reg_c);
      ea = (val_a & ~0x3);
      if (!(beta->pc & (1<<31)) && (ea & (1<<31))) {
        ea &= 0x7FFFFFFF;
      }
      beta->pc = ea;
      break;
    case LD:
      beta->pc += 4;
      ea = (val_a&~(1<<31)) + (int32_t)literal;
      beta_write_reg(beta, beta_read_mem(beta, ea), reg_c);
      break;
    case LDR:
      beta->pc += 4;
      ea = (beta->pc & ~(1 << 31)) + ((int32_t)literal) * 4;
      beta_write_reg(beta, beta_read_mem(beta, ea), reg_c);
      break;
    case MUL:
      beta_write_reg(beta, val_a * val_b, reg_c);
      beta->pc += 4;
      break;
    case MULC:
      beta_write_reg(beta, val_a * literal, reg_c);
      beta->pc += 4;
      break;
    case OR:
      beta_write_reg(beta, val_a | val_b, reg_c);
      beta->pc += 4;
      break;
    case ORC:
      beta_write_reg(beta, val_a | literal, reg_c);
      beta->pc += 4;
      break;
    case SHL:
      beta_write_reg(beta, val_a << val_b, reg_c);
      beta->pc += 4;
      break;
    case SHLC:
      beta_write_reg(beta, val_a << literal, reg_c);
      beta->pc += 4;
      break;
    case SHR:
      beta_write_reg(beta, val_a >> val_b, reg_c);
      beta->pc += 4;
      break;
    case SHRC:
      beta_write_reg(beta, val_a >> literal, reg_c);
      beta->pc += 4;
      break;
    case SRA:
      res = val_a >> val_b;

      if (val_a & (1 << 31)) {
        res |= 0xFFFFFFFF << (32 - val_b);
      }

      beta_write_reg(beta, res, reg_c);
      beta->pc += 4; 
      break;
    case SRAC:
      res = val_a >> literal;

      if (val_a & (1 << 31)) {
        res |= 0xFFFFFFFF << (32 - literal);
      }

      beta_write_reg(beta, res, reg_c);
      beta->pc += 4;
      break;
    case SUB:
      beta_write_reg(beta, val_a - val_b, reg_c);
      beta->pc += 4;
      break;
    case SUBC:
      beta_write_reg(beta, val_a - literal, reg_c);
      beta->pc += 4;
      break;
    case ST:
      ea = (val_a&~(1<<31)) + (int32_t)literal;
      beta_write_mem(beta, val_c, ea);
      beta->pc += 4;
      break;
    case XOR:
      beta_write_reg(beta, val_a ^ val_b, reg_c);
      beta->pc += 4;
      break;
    case XORC:
      beta_write_reg(beta, val_a ^ literal, reg_c);
      beta->pc += 4;
      break;
    case XNOR:
      beta_write_reg(beta, ~(val_a ^ val_b), reg_c);
      beta->pc += 4;
      break;
    case XNORC:
      beta_write_reg(beta, ~(val_a ^ literal), reg_c);
      beta->pc += 4;
      break;

    case 0: // special opcodes
      switch (literal) {
        case X_HALT:
          beta->halted = true;
          break;
        case X_RDCHAR:
          beta->registers[0] = beta->key;
          break;
        case X_WRCHAR:
          c = (char)beta->registers[0];
          term_putc(beta->tty, c);
          break;
        case X_CYCLE:
          break;
        case X_TIME:
          break;
        case X_CLICK:
          break;
        case X_RANDOM:
          break;
        case X_SEED:
          break;
        case X_SERVER:
          break;
        case X_GR_MEM:
          beta->registers[0] = (beta->graph_mem - beta->memory)*4;
          break;
      }

      beta->pc += 4;
      break;

    default: // Illegal opcode
      beta_write_reg(beta, beta->pc + 4, REG_XP);
      beta->pc = 4 | (1 << 31);
      break;
  }

  // Call lua hook
  lua_getglobal(L, "tick");
  lua_pushnumber(L, last_pc);
  lua_pushnumber(L, opcode);
  lua_pushnumber(L, reg_c);
  lua_pushnumber(L, reg_a);
  lua_pushnumber(L, reg_b);
  lua_pushnumber(L, literal);

  if (lua_pcall(L, 6, 0, 0) != 0) {
    //error(L, "error running function `f': %s", lua_tostring(L, -1));
    printf("Lua error: %s\n", lua_tostring(L, -1));
    exit(1);
  }

  last_pc = beta->pc;
}
