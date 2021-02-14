#include <efi.h>
#include <efilib.h>
#include <elf.h>
#include "util.c"

EFI_STATUS efi_main (EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
	InitializeLib(ImageHandle, SystemTable);
	Print(L"Hello World!\n\r");

	EFI_FILE* kernel = load_file(NULL, L"foxkrnl.elf", ImageHandle, SystemTable);
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

	int (*kernel_start)() = ((__attribute__((sysv_abi)) int (*)() ) header.e_entry);

	Print(L"%d\r\n", kernel_start());

	return EFI_SUCCESS;
}
