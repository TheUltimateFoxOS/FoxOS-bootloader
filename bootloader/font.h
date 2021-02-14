#ifndef FONT_H
#define FONT_H

#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

typedef struct {
	unsigned char magic[2];
	unsigned char mode;
	unsigned char charsize;
} psf1_header_t;

typedef struct {
	psf1_header_t* psf1_Header;
	void* glyph_buffer;
} psf1_font_t;

#endif