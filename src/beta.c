#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "beta.h"

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

void beta_dump_info(Beta* beta) {
	printf("PC = %d (%x)\n", beta->pc, beta->pc);
}

void beta_dump_registers(Beta* beta) {
	for(int i=0; i < 31; i++) {
		printf("R%d\t= %x\n", i, beta->registers[i]);
	}
}

void beta_dump_memory(Beta* beta) {
	for(int i=0; i < beta->memsize; i++)
		printf("%x) %x\n", i, beta->memory[i]);
}

Beta* beta_create(int memsize) {
	Beta* newbeta = calloc(1, sizeof(Beta));
	uint32_t* memory = calloc(memsize, sizeof(uint32_t));

	newbeta->memsize = memsize;
	newbeta->memory = memory;

	return newbeta;
}

uint32_t beta_read_reg(Beta* beta, uint8_t index) {
	index &= 0x1F;	// register indices are 5 bits

	if(index == 31)
		return 0;
	else
		return beta->registers[index];
}

void beta_write_reg(Beta* beta, uint32_t value, uint8_t index) {
	//printf("R%d = 0x%x\n", index, value);
	index &= 0x1F;	// register indices are 5 bits
	if(index != 31) beta->registers[index] = value;
}

uint32_t beta_read_mem(Beta* beta, uint32_t addr) {
	addr >>= 2;

	if(addr < beta->memsize)
		return beta->memory[addr];
	else
		printf("read: %x out of bounds\n", addr);
	return 0;
}

void beta_write_mem(Beta* beta, uint32_t value, uint32_t addr) {
	addr >>= 2;

	if(addr < beta->memsize)
		beta->memory[addr] = value;
	else
		printf("write: %x out of bounds\n", addr);
}

void beta_interrupt(Beta* beta, uint32_t address) {
	beta_write_reg(beta, beta->pc + 4, REG_XP);
	beta->pc = address | (1 << 31);
	beta_tick(beta);
}

const char* decode(uint32_t op) {
	switch(op) {
		case ADD:	return "ADD";
		case ADDC:	return "ADDC";
		case AND:	return "AND";
		case ANDC:	return "ANDC";
		case BEQ:	return "BEQ";
		case BNE:	return "BNE";
		case CMPEQ:	return "CMPEQ";
		case CMPEQC:return "CMPEQC";
		case CMPLE:	return "CMPLE";
		case CMPLEC:return "CMPLEC";
		case CMPLT:	return "CMPLT";
		case CMPLTC:return "CMPLTC";
		case DIV:	return "DIV";
		case DIVC:	return "DIVC";
		case JMP:	return "JMP";
		case LD:	return "LD";
		case LDR:	return "LDR";
		case MUL:	return "MUL";
		case MULC:	return "MULC";
		case OR:	return "OR";
		case ORC:	return "ORC";
		case SHL:	return "SHL";
		case SHLC:	return "SHLC";
		case SHR:	return "SHR";
		case SHRC:	return "SHRC";
		case SRA:	return "SRA";
		case SRAC:	return "SRAC";
		case SUB:	return "SUB";
		case SUBC:	return "SUBC";
		case ST:	return "ST";
		case XOR:	return "XOR";
		case XORC:	return "XORC";
		case XNOR:	return "XNOR";
		case XNORC:	return "XNORC";
		default:
			return "???";
	}
}

void beta_tick(Beta* beta) {
	if(beta->halted) return;

	// get next instruction
	uint32_t instruction = beta_read_mem(beta, beta->pc & ~(1 << 31));

	// decode instruction fields
	uint8_t opcode		= instruction >> 26;
	uint8_t reg_c		= (instruction >> (26-5)) & 0x1F;
	uint8_t reg_a		= (instruction >> (26-10)) & 0x1F;
	uint8_t reg_b		= (instruction >> (26-15)) & 0x1F;
	int16_t literal		= instruction & 0xFFFF;

	if(beta->pc & (1<<31))
		printf("SUPER [%d (%x)]\t%s\t%d, %d, %d / %d\n", beta->pc & ~(1<<31), beta->pc & ~(1<<31), decode(opcode), reg_a, reg_b, reg_c, literal);
	else
		printf("user  [%d (%x)]\t%x\t%d, %d, %d / %d\n", beta->pc & ~(1<<31), beta->pc & ~(1<<31), opcode, reg_a, reg_b, reg_c, literal);

	// get data
	uint32_t val_c	= beta_read_reg(beta, reg_c);
	uint32_t val_a	= beta_read_reg(beta, reg_a);
	uint32_t val_b	= beta_read_reg(beta, reg_b);

	// execute instruction
	uint32_t res, ea;
	char c;
	switch(opcode) {
		case ADD:	beta_write_reg(beta, val_a + val_b, reg_c);		break;
		case ADDC:	beta_write_reg(beta, val_a + literal, reg_c);	break;
		case AND:	beta_write_reg(beta, val_a & val_b, reg_c);		break;
		case ANDC:	beta_write_reg(beta, val_a & literal, reg_c);	break;
		case BEQ:
			beta_write_reg(beta, beta->pc+4, reg_c);
			ea = beta->pc + ((int32_t)literal) * 4;
			if(val_a == 0) beta->pc = ea;
			break;
		case BNE:
			beta_write_reg(beta, beta->pc+4, reg_c);
			ea = beta->pc + ((int32_t)literal) * 4;
			if(val_a != 0) beta->pc = ea;
			break;
		case CMPEQ:
			beta_write_reg(beta, (val_a == val_b)? 1: 0, reg_c);
			break;
		case CMPEQC:
			beta_write_reg(beta, ((int32_t)val_a == (int32_t)literal)? 1: 0, reg_c);
			break;
		case CMPLE:
			beta_write_reg(beta, ((int32_t)val_a <= (int32_t)val_b)? 1: 0, reg_c);
			break;
		case CMPLEC: break;
			beta_write_reg(beta, ((int32_t)val_a <= (int32_t)literal)? 1: 0, reg_c);
			break;
		case CMPLT:
			beta_write_reg(beta, ((int32_t)val_a < (int32_t)val_b)? 1: 0, reg_c);
			break;
		case CMPLTC:
			beta_write_reg(beta, ((int32_t)val_a < (int32_t)literal)? 1: 0, reg_c);
			break;
		case DIV:	beta_write_reg(beta, val_a / val_b, reg_c);		break;
		case DIVC:	beta_write_reg(beta, val_a / literal, reg_c);	break;
		case JMP:
			beta_write_reg(beta, beta->pc+4, reg_c);
			ea = (val_a & 0xFFFFFFFC) - 4;
			if(!(beta->pc & (1<<31)) && (ea & (1<<31))) ea &= 0x3FFFFFFF;
			beta->pc = ea;
			break;
		case LD:
			ea = val_a + (int32_t)literal;
			beta_write_reg(beta, beta_read_mem(beta, ea), reg_c);
			break;
		case LDR:
			ea = ((beta->pc + 4) & ~(1 << 31)) + ((int32_t)literal) * 4;
			beta_write_reg(beta, beta_read_mem(beta, ea), reg_c);
			break;
		case MUL:	beta_write_reg(beta, val_a * val_b, reg_c);		break;
		case MULC:	beta_write_reg(beta, val_a * literal, reg_c);	break;
		case OR:	beta_write_reg(beta, val_a | val_b, reg_c);		break;
		case ORC:	beta_write_reg(beta, val_a | literal, reg_c);	break;
		case SHL:	beta_write_reg(beta, val_a << val_b, reg_c);	break;
		case SHLC:	beta_write_reg(beta, val_a << literal, reg_c);	break;
		case SHR:	beta_write_reg(beta, val_a >> val_b, reg_c);	break;
		case SHRC:	beta_write_reg(beta, val_a >> literal, reg_c);	break;
		case SRA:
			res = val_a >> val_b;

			if(val_a & (1 << 31))
				res |= 0xFFFFFFFF << (32 - val_b);

			beta_write_reg(beta, res, reg_c);

			break;
		case SRAC:
			res = val_a >> literal;

			if(val_a & (1 << 31))
				res |= 0xFFFFFFFF << (32 - literal);

			beta_write_reg(beta, res, reg_c);

			break;
		case SUB:	beta_write_reg(beta, val_a - val_b, reg_c);			break;
		case SUBC:	beta_write_reg(beta, val_a - literal, reg_c);		break;
		case ST:
			ea = val_a + (int32_t)literal;
			beta_write_mem(beta, val_c, ea);
			break;
		case XOR:	beta_write_reg(beta, val_a ^ val_b, reg_c);			break;
		case XORC:	beta_write_reg(beta, val_a ^ literal, reg_c);		break;
		case XNOR:	beta_write_reg(beta, ~(val_a ^ val_b), reg_c);		break;
		case XNORC: beta_write_reg(beta, ~(val_a ^ literal), reg_c);	break;

		case 0: // special opcodes
			switch(literal) {
				case X_HALT:
					printf("beta: halt.\n");
					beta->halted = true;
					break;
				case X_RDCHAR:
					printf("RDCHAR\n");
					beta->registers[0] = beta->key;
					break;
				case X_WRCHAR:
					c = (char)beta->registers[0];
					term_putc(beta->tty, c);
					break;
				case X_CYCLE:
					printf("beta: cycle.\n");
					break;
				case X_TIME:
					printf("beta: time.\n");
					break;
				case X_CLICK:
					printf("beta: click.\n");
					break;
				case X_RANDOM:
					printf("beta: random.\n");
					break;
				case X_SEED:
					printf("beta: seed.\n");
					break;
				case X_SERVER:
					printf("beta: server ping.\n");
					break;
			}
			break;

		default: // illegal opcode
			beta_write_reg(beta, beta->pc + 4, REG_XP);
			beta->pc = 0 | (1 << 31);
			break;
	}

	// move on
	beta->pc += 4;
}
