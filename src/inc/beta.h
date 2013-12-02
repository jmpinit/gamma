#ifndef BETA_H
#define BETA_H

#include <stdbool.h>
#include <stdint.h>

#include "term.h"

#define REG_XP		30
#define REG_ZERO	31

#define VEC_RESET	0
#define VEC_II		4	// illegal instruction (also SVC call)
#define VEC_CLK		8	// clock interrupt
#define	VEC_KBD		12	// keyboard interrupt
#define VEC_MOUSE	16	// mouse interrupt

#define X_HALT 		0
#define X_RDCHAR	1
#define X_WRCHAR	2
#define X_CYCLE		3
#define X_TIME		4
#define X_CLICK		5
#define X_RANDOM	6
#define X_SEED		7
#define X_SERVER	8

extern Terminal* terminal;

typedef enum {
	ADD		= 0x20,
	ADDC	= 0x30,
	AND		= 0x28,
	ANDC	= 0x38,
	BEQ		= 0x1C,
	BNE		= 0x1D,
	CMPEQ	= 0x24,
	CMPEQC	= 0x34,
	CMPLE	= 0x26,
	CMPLEC	= 0x36,
	CMPLT	= 0x25,
	CMPLTC	= 0x35,
	DIV		= 0x23,
	DIVC	= 0x33,
	JMP		= 0x1B,
	LD		= 0x18,
	LDR		= 0x1F,
	MUL		= 0x22,
	MULC	= 0x32,
	OR		= 0x29,
	ORC		= 0x39,
	SHL		= 0x2C,
	SHLC	= 0x3C,
	SHR		= 0x2D,
	SHRC	= 0x3D,
	SRA		= 0x2E,
	SRAC	= 0x3E,
	SUB		= 0x21,
	SUBC	= 0x31,
	ST		= 0x19,
	XOR		= 0x2A,
	XORC	= 0x3A,
	XNOR	= 0x2B,
	XNORC	= 0x3B
} Instruction;

typedef struct Beta {
	bool halted;
	unsigned int memsize;
	uint32_t* memory;
	uint32_t pc;
	uint32_t registers[31];

	Terminal* tty;
	char key;
} Beta;

extern Beta* beta;

// simulation
Beta*		beta_create(int memsize);
void		beta_tick(Beta*, lua_State *L);
void		beta_interrupt(Beta*, lua_State *L, uint32_t address);
uint32_t	beta_read_reg(Beta*, uint8_t index);
void		beta_write_reg(Beta*, uint32_t value, uint8_t index);
uint32_t	beta_read_mem(Beta*, uint32_t addr);
void		beta_write_mem(Beta*, uint32_t value, uint32_t addr);

// control
void		beta_load(Beta*, const char* filename);

// debug
void		beta_dump_info(Beta*);
void		beta_dump_registers(Beta*);

#endif
