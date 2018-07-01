#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <core/logging.h>
#include <core/elf.h>
#include <algorithms/linkedlist.h>

static const char* ELF_ERROR_MSG[] = {
    NULL, // ELF_SUCCESS
    "not an ELF format", // ELF_ERROR_NO_ELF
    "invalid version", // ELF_ERROR_INVALID_ELF_VERSION
    "incompatible architecture", // ELF_ERROR_INCOMPATIBLE
};

#define THROW_ERROR(code)         log_error(ELF_ERROR_MSG[code]); return code;

int elf_parser(void* raw_elf, size_t size, elf_t* elf)
{
    if (raw_elf == NULL || size < 50) {
        THROW_ERROR(ELF_ERROR_NO_ELF);
    }

    elf_ident_t* ident = (elf_ident_t*) raw_elf;
    // validates ELF Magic number
    if (!(ident->magic[0] == 0x7f && ident->magic[1] == 0x45 && ident->magic[2] == 0x4c && ident->magic[3] == 0x46)) {
        THROW_ERROR(ELF_ERROR_NO_ELF);
    }

    // validates version
    if (ident->ident_version != ELF_VALID_IDENT_VERSION || ident->elf_version != ELF_VALID_ELF_VERSION) {
        THROW_ERROR(ELF_ERROR_INVALID_ELF_VERSION);
    }

    if (ident->endian != ELF_VALID_ENDIAN || ident->elfclass != ELF_VALID_CLASS || ident->os_abi != ELF_VALID_OSABI ||
        ident->machine != ELF_VALID_MACHINE) {
        THROW_ERROR(ELF_ERROR_INCOMPATIBLE);
    }

    elf->base = (uintptr_t) raw_elf;
    elf->ident = (elf_ident_t*) elf->base;
    // assuming only 64 bits ELF
    elf->entry_point = *((uintptr_t*) (raw_elf + 0x18));
    elf->flags = *((uint32_t*) (raw_elf + 0x30));

    size_t program_header_entries = *((uint16_t*) (raw_elf + 0x38));
    uintptr_t program_header_next_address = *((uintptr_t*) (raw_elf + 0x20));
    program_header_next_address += elf->base;
    elf->program_headers = linkedlist_create();
    for (size_t entry=0; entry < program_header_entries; ++entry) {
        linkedlist_append(elf->program_headers, (void*) program_header_next_address);
        program_header_next_address += sizeof(elf_program_header_t);
    }
    return ELF_SUCCESS;
}

void elf_release(elf_t* elf)
{
    if (!elf) {
        return;
    }
    // since the headers are technically only pointers, it is not necessary deallocates their memory
    linkedlist_destroy(elf->program_headers);
}
