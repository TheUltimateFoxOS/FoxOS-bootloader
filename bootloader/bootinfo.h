#ifndef BOOTINFO_H
#define BOOTINFO_H

typedef struct {
	psf1_font_t* font;
	framebuffer_t* framebuffer;
	EFI_MEMORY_DESCRIPTOR* m_map;
	UINTN m_map_size;
	UINTN m_map_desc_size;
} bootinfo_t;

#endif