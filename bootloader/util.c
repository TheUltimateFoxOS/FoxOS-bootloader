UINTN strcmp(CHAR8* a, CHAR8* b, UINTN length){
	for (UINTN i = 0; i < length; i++){
		if (*a != *b) return 0;
	}
	return 1;
}

EFI_FILE* load_file(EFI_FILE* dir, CHAR16* path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable){
	EFI_FILE* loaded_file;

	EFI_LOADED_IMAGE_PROTOCOL* loaded_image;
	SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (void**)&loaded_image);

	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system;
	SystemTable->BootServices->HandleProtocol(loaded_image->DeviceHandle, &gEfiSimpleFileSystemProtocolGuid, (void**)&file_system);

	if (dir == NULL){
		file_system->OpenVolume(file_system, &dir);
	}

	EFI_STATUS s = dir->Open(dir, &loaded_file, path, EFI_FILE_MODE_READ, EFI_FILE_READ_ONLY);
	if (s != EFI_SUCCESS){
		return NULL;
	}
	return loaded_file;

}

int memcmp(const void* aptr, const void* bptr, size_t n) {
	const unsigned char* a = aptr, *b = bptr;
	for (size_t i = 0; i < n; i++){
		if (a[i] < b[i]) return -1;
		else if (a[i] > b[i]) return 1;
	}
	return 0;
}

framebuffer_t framebuffer;

framebuffer_t* initialize_gop(){
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
	EFI_STATUS status;

	status = uefi_call_wrapper(BS->LocateProtocol, 3, &gopGuid, NULL, (void**)&gop);
	if(EFI_ERROR(status)){
		return NULL;
	}

	framebuffer.base_addres = (void*)gop->Mode->FrameBufferBase;
	framebuffer.buffer_size = gop->Mode->FrameBufferSize;
	framebuffer.width = gop->Mode->Info->HorizontalResolution;
	framebuffer.height = gop->Mode->Info->VerticalResolution;
	framebuffer.pixels_per_scanline = gop->Mode->Info->PixelsPerScanLine;

	return &framebuffer;
	
}


void print_framebuffer_info(framebuffer_t* buffer) {
	Print(L"GOP located:\n\r");
	Print(L"    Base addres: 0x%x\n\r", buffer->base_addres);
	Print(L"    Buffer size: %d\n\r", buffer->buffer_size);
	Print(L"    Width: %d\n\r", buffer->width);
	Print(L"    Height: %d\n\r", buffer->height);
	Print(L"    Pixels per scanline: %d\n\r", buffer->pixels_per_scanline);
}

psf1_font_t* load_psf1_font(EFI_FILE* Directory, CHAR16* Path, EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	EFI_FILE* font = load_file(Directory, Path, ImageHandle, SystemTable);
	if (font == NULL) return NULL;

	psf1_header_t* font_header;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(psf1_header_t), (void**)&font_header);
	UINTN size = sizeof(psf1_header_t);
	font->Read(font, &size, font_header);

	if (font_header->magic[0] != PSF1_MAGIC0 || font_header->magic[1] != PSF1_MAGIC1){
		return NULL;
	}

	UINTN glyph_buffer_size = font_header->charsize * 256;
	if (font_header->mode == 1) {
		glyph_buffer_size = font_header->charsize * 512;
	}

	void* glyph_buffer;
	{
		font->SetPosition(font, sizeof(psf1_header_t));
		SystemTable->BootServices->AllocatePool(EfiLoaderData, glyph_buffer_size, (void**)&glyph_buffer);
		font->Read(font, &glyph_buffer_size, glyph_buffer);
	}

	psf1_font_t* finished_font;
	SystemTable->BootServices->AllocatePool(EfiLoaderData, sizeof(psf1_font_t), (void**)&finished_font);
	finished_font->psf1_Header = font_header;
	finished_font->glyph_buffer = glyph_buffer;
	return finished_font;

}