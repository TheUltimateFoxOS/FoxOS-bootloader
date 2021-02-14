#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include <stddef.h>
#include "framebuffer.h"
#include "font.h"
#include "bootinfo.h"
#include "util.c"

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);
	Print(L"Hello World!\n\r");

	EFI_FILE* kernel = load_file(NULL, L"EFI\\FOXOS\\foxkrnl.elf", ImageHandle, SystemTable);
	if(kernel == NULL) {
		Print(L"Kernel load error\n\r");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Kernel load success\n\r");
	}

	Elf64_Ehdr header;
	{
		UINTN file_info_size;
		EFI_FILE_INFO* file_info;
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_info_size, NULL);
		SystemTable->BootServices->AllocatePool(EfiLoaderData, file_info_size, (void**) &file_info_size);
		kernel->GetInfo(kernel, &gEfiFileInfoGuid, &file_info_size, (void**) &file_info);

		UINTN size = sizeof(header);
		kernel->Read(kernel, &size, &header);
	}

	if (memcmp(&header.e_ident[EI_MAG0], ELFMAG, SELFMAG) != 0 || header.e_ident[EI_CLASS] != ELFCLASS64 || header.e_ident[EI_DATA] != ELFDATA2LSB || header.e_type != ET_EXEC || header.e_machine != EM_X86_64 || header.e_version != EV_CURRENT) {
		Print(L"Kernel format bad\n\r");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"kernel header successfully verified\n\r");
	}

	Elf64_Phdr* phdrs;
	{
		kernel->SetPosition(kernel, header.e_phoff);
		UINTN size = header.e_phnum * header.e_phentsize;
		SystemTable->BootServices->AllocatePool(EfiLoaderData, size, (void**) &phdrs);
		kernel->Read(kernel, &size, phdrs);
	}

	for (Elf64_Phdr* phdr = phdrs; (char*)phdr < (char*)phdrs + header.e_phnum * header.e_phentsize; phdr = (Elf64_Phdr*)((char*)phdr + header.e_phentsize)) {
		switch(phdr->p_type) {
			case PT_LOAD:
			{
				int pages = (phdr->p_memsz + 0x1000 - 1) / 0x1000;
				Elf64_Addr segment = phdr->p_paddr;
				SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, pages, &segment);

				kernel->SetPosition(kernel, phdr->p_offset);
				UINTN size = phdr->p_filesz;
				kernel->Read(kernel, &size, (void*) segment);
				break;
			}
		}
	}

	Print(L"Kernel loaded\n\r");

	void (*kernel_start)(bootinfo_t*) = ((__attribute__((sysv_abi)) void (*)(bootinfo_t*) ) header.e_entry);

	framebuffer_t* buffer = initialize_gop();

	if(buffer == NULL) {
		Print(L"Unable to locate GOP\n\r");
		return EFI_DEVICE_ERROR;
	} else {
		print_framebuffer_info(buffer);
	}

	psf1_font_t* font = load_psf1_font(NULL, L"EFI\\FOXOS\\zap-light16.psf", ImageHandle, SystemTable);

	if(font == NULL) {
		Print(L"Font load error\n\r");
		return EFI_LOAD_ERROR;
	} else {
		Print(L"Font load success\n\r");
	}

	bootinfo_t bootinfo;
	bootinfo.framebuffer = buffer;
	bootinfo.font = font;

	kernel_start(&bootinfo);

	return EFI_SUCCESS;
}
