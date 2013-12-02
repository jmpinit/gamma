#ifndef TERM_H
#define TERM_H

#include "SDL/SDL.h"

typedef struct Terminal {
	unsigned int left, top;
	unsigned int width, height;
	unsigned int x, y;
	char* text;
	SDL_Surface* font;
} Terminal;

Terminal* term_create(unsigned int x, unsigned int y, unsigned int w, unsigned int h, const char* fn);

void term_clear(Terminal* t);
void term_move(Terminal* t, unsigned int x, unsigned int y);
void term_next(Terminal* t);
void term_putc(Terminal* t, char c);
void term_puts(Terminal* t, const char* str);

void term_render(Terminal* t, struct SDL_Surface* canvas);

#endif
