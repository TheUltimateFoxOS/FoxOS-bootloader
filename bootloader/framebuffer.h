#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

typedef struct {
	void* base_addres;
	size_t buffer_size;
	unsigned int width;
	unsigned int height;
	unsigned int pixels_per_scanline;
} framebuffer_t;

#endif