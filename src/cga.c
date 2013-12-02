#include "SDL/SDL.h"

#include "cga.h"

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

CGA* cga_create(uint w, uint h, uint pixelsize) {
	CGA* newcga = calloc(1, sizeof(CGA));
	uint8_t* pixels = calloc(w*h/2, sizeof(uint8_t));

	newcga->width = w;
	newcga->height = h;
	newcga->pixelsize = pixelsize;
	newcga->pixels = pixels;

	return newcga;
}

void cga_render(CGA* adapter, SDL_Surface* canvas, uint left, uint top) {
	uint i = 0;
	for(uint y=0; y < adapter->height; y++) { 
		for(uint x=0; x < adapter->width; x++) { 
			uint8_t index = adapter->pixels[i/2];
			i++;

			// get correct nibble
			if(x%2==0)
				index &= 0xF;
			else
				index >>= 4;

			for(uint oy=0; oy < adapter->pixelsize; oy++)
				for(uint ox=0; ox < adapter->pixelsize; ox++)
					set_pixel(canvas, left + x*adapter->pixelsize + ox, top + y*adapter->pixelsize + oy, cga_colors[index]);
		}
	}
}

void cga_set(CGA* s, uint x, uint y, uint8_t v) {

}

void cga_get(CGA* s, uint x, uint y, uint8_t v) {

}

void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
	int bpp = surface->format->BytesPerPixel;

	// calculate address of pixel
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

	switch(bpp) {
		case 1:
			*p = pixel;
			break;

		case 2:
			*(Uint16 *)p = pixel;
			break;

		case 3:
			if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				p[0] = (pixel >> 16) & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = pixel & 0xff;
			} else {
				p[0] = pixel & 0xff;
				p[1] = (pixel >> 8) & 0xff;
				p[2] = (pixel >> 16) & 0xff;
			}
			break;

		case 4:
			*(Uint32 *)p = pixel;
			break;
	}
}
