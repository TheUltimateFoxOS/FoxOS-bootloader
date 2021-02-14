#include <stddef.h>

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