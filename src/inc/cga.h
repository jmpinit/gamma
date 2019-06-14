#ifndef CGA_H
#define CGA_H

#include "SDL2/SDL.h"

typedef enum {
	BLACK 			= 0,
	BLUE			= 0x0000AA,
	GREEN			= 0x00AA00,
	CYAN			= 0x00AAAA,
	RED				= 0xAA0000,
	MAGENTA			= 0xAA00AA,
	BROWN			= 0xAA5500,
	LIGHT_GRAY		= 0xAAAAAA,
	GRAY			= 0x555555,
	LIGHT_BLUE		= 0x5555FF,
	LIGHT_GREEN		= 0x55FF55,
	LIGHT_CYAN		= 0x55FFFF,
	LIGHT_RED		= 0xFF5555,
	LIGHT_MAGENTA	= 0xFF55FF,
	YELLOW			= 0xFFFF55,
	WHITE			= 0xFFFFFF
} Color;

static const uint32_t cga_colors[] = {
	BLACK,			BLUE,
	GREEN,			CYAN,
	RED,			MAGENTA,
	BROWN,			LIGHT_GRAY,
	GRAY,			LIGHT_BLUE,
	LIGHT_GREEN,	LIGHT_CYAN,
	LIGHT_RED,		LIGHT_MAGENTA,
	YELLOW,			WHITE
};

typedef struct CGA {
	uint width, height;
	uint pixelsize;
	uint8_t* pixels;
} CGA;

uint cga_byte_size(CGA*);

CGA* cga_create(uint w, uint h, uint pixelsize);
void cga_render(CGA* adapter, SDL_Surface* s, uint x, uint y);

void cga_set(CGA* s, uint x, uint y, uint8_t v);
uint8_t cga_get(CGA* s, uint x, uint y);

#endif
