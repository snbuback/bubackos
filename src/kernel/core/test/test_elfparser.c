// source: kernel/c/loader/elfparser.c
// source: libk/c/algorithms/linkedlist.c
#include <kernel_test.h>
#include <loader/elf.h>
#include <stdio.h>
#include <string.h>
#include <algorithms/linkedlist.h>

static char* raw_elf;
static size_t raw_elf_size;
static const char* ELF_TEST_FILE = "src/kernel/test/loader/hello.elf";

void setUp(void)
{
    FILE* f = fopen(ELF_TEST_FILE, "rb");
    if (!f) {
        printf("Invalid file: %s\n", ELF_TEST_FILE);
        // probably next call will generate null pointer
        return;
    }

    // size of file
    fseek (f , 0 , SEEK_END);
    raw_elf_size = ftell(f);
    rewind(f);

    raw_elf = (char*) malloc(raw_elf_size);
    raw_elf_size = fread(raw_elf, 1, raw_elf_size, f);
    fclose(f);
}

void tearDown(void)
{
    free(raw_elf);
    raw_elf = NULL;
    raw_elf_size = 0;
}

void test_no_elf() {
    elf_t* elf = (elf_t*) 0x0; // to cause null pointer in case of access even when there is no ELF
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_NO_ELF, elf_parser(NULL, 0, elf));
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_NO_ELF, elf_parser((void*) 0x100, 49, elf));
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_NO_ELF, elf_parser("xpaot", 500, elf));
}

void test_invalid_elf_ident_version() {
    elf_t elf;
    raw_elf[0x6] = 2; // change to version 2
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INVALID_ELF_VERSION, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_invalid_elf_elf_version() {
    elf_t elf;
    raw_elf[0x14] = 2; // change to version 2
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INVALID_ELF_VERSION, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_incompatible_elf_endian() {
    elf_t elf;
    raw_elf[0x5] = 2; // change to big-endian
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INCOMPATIBLE, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_incompatible_elf_class() {
    elf_t elf;
    raw_elf[0x4] = 1; // change to version 32 bits
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INCOMPATIBLE, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_incompatible_elf_os() {
    elf_t elf;
    raw_elf[0x7] = 1; // change to hp-ux
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INCOMPATIBLE, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_incompatible_elf_machine() {
    elf_t elf;
    raw_elf[0x12] = 1;
    TEST_ASSERT_EQUAL_INT(ELF_ERROR_INCOMPATIBLE, elf_parser(raw_elf, raw_elf_size, &elf));
}

void test_parse_header() {
    elf_t elf;
    memset(&elf, 0, sizeof(elf_t));
    TEST_ASSERT_EQUAL(ELF_SUCCESS, elf_parser(raw_elf, raw_elf_size, &elf));
    TEST_ASSERT_EQUAL(ELF_VALID_CLASS, elf.ident->elfclass);
    TEST_ASSERT_EQUAL(ELF_VALID_ENDIAN, elf.ident->endian);
    TEST_ASSERT_EQUAL(ELF_VALID_OSABI, elf.ident->os_abi);
    TEST_ASSERT_EQUAL(ELF_ET_EXEC, elf.ident->type);
    TEST_ASSERT_EQUAL(ELF_VALID_MACHINE, elf.ident->machine);
    TEST_ASSERT_EQUAL_HEX64(0x40019f, elf.entry_point);
    TEST_ASSERT_EQUAL_HEX64(0x0, elf.flags);

    // program header
    TEST_ASSERT_NOT_NULL(elf.program_headers);
    TEST_ASSERT_EQUAL(5, linkedlist_size(elf.program_headers));

    elf_program_header_t* ph;
    //Type           Offset   VirtAddr           PhysAddr           FileSiz  MemSiz   Flg Align
    //PHDR           0x000040 0x0000000000400040 0x0000000000400040 0x000118 0x000118 R   0x8
    ph = linkedlist_get(elf.program_headers, 0);
    TEST_ASSERT_EQUAL(ELF_PT_PHDR, ph->type);
    TEST_ASSERT_EQUAL_HEX64(0x40, ph->offset);
    TEST_ASSERT_EQUAL_HEX64(0x400040, ph->vaddr);
    TEST_ASSERT_EQUAL_HEX64(0x400040, ph->paddr);
    TEST_ASSERT_EQUAL_HEX64(0x118, ph->file_size);
    TEST_ASSERT_EQUAL_HEX64(0x118, ph->mem_size);
    TEST_ASSERT_EQUAL_HEX64(0x8, ph->align);
    TEST_ASSERT_EQUAL(ELF_PF_FLAGS_R, ph->flags);

    //INTERP         0x0001f8 0x00000000004001f8 0x00000000004001f8 0x00000e 0x00000e R   0x8
    ph = linkedlist_get(elf.program_headers, 1);
    TEST_ASSERT_EQUAL(ELF_PT_INTERP, ph->type);
    TEST_ASSERT_EQUAL_HEX64(0x1f8, ph->offset);
    TEST_ASSERT_EQUAL_HEX64(0x4001f8, ph->vaddr);
    TEST_ASSERT_EQUAL_HEX64(0x4001f8, ph->paddr);
    TEST_ASSERT_EQUAL_HEX64(0xe, ph->file_size);
    TEST_ASSERT_EQUAL_HEX64(0xe, ph->mem_size);
    TEST_ASSERT_EQUAL_HEX64(0x8, ph->align);
    TEST_ASSERT_EQUAL(ELF_PF_FLAGS_R, ph->flags);

    //LOAD           0x000000 0x0000000000400000 0x0000000000400000 0x0001f6 0x0001f6 R E 0x1000
    ph = linkedlist_get(elf.program_headers, 2);
    TEST_ASSERT_EQUAL(ELF_PT_LOAD, ph->type);
    TEST_ASSERT_EQUAL_HEX64(0x0, ph->offset);
    TEST_ASSERT_EQUAL_HEX64(0x400000, ph->vaddr);
    TEST_ASSERT_EQUAL_HEX64(0x400000, ph->paddr);
    TEST_ASSERT_EQUAL_HEX64(0x1f6, ph->file_size);
    TEST_ASSERT_EQUAL_HEX64(0x1f6, ph->mem_size);
    TEST_ASSERT_EQUAL_HEX64(0x1000, ph->align);
    TEST_ASSERT_EQUAL(ELF_PF_FLAGS_R | ELF_PF_FLAGS_X, ph->flags);

    //LOAD           0x0001f8 0x00000000004001f8 0x00000000004001f8 0x0000a0 0x0000a0 R   0x1000
    ph = linkedlist_get(elf.program_headers, 3);
    TEST_ASSERT_EQUAL(ELF_PT_LOAD, ph->type);
    TEST_ASSERT_EQUAL_HEX64(0x1f8, ph->offset);
    TEST_ASSERT_EQUAL_HEX64(0x4001f8, ph->vaddr);
    TEST_ASSERT_EQUAL_HEX64(0x4001f8, ph->paddr);
    TEST_ASSERT_EQUAL_HEX64(0xa0, ph->file_size);
    TEST_ASSERT_EQUAL_HEX64(0xa0, ph->mem_size);
    TEST_ASSERT_EQUAL_HEX64(0x1000, ph->align);
    TEST_ASSERT_EQUAL(ELF_PF_FLAGS_R, ph->flags);

    //LOAD           0x000298 0x0000000000400298 0x0000000000400298 0x000004 0x0001cc RW  0x1000
    ph = linkedlist_get(elf.program_headers, 4);
    TEST_ASSERT_EQUAL(ELF_PT_LOAD, ph->type);
    TEST_ASSERT_EQUAL_HEX64(0x298, ph->offset);
    TEST_ASSERT_EQUAL_HEX64(0x400298, ph->vaddr);
    TEST_ASSERT_EQUAL_HEX64(0x400298, ph->paddr);
    TEST_ASSERT_EQUAL_HEX64(0x4, ph->file_size);
    TEST_ASSERT_EQUAL_HEX64(0x1cc, ph->mem_size);
    TEST_ASSERT_EQUAL_HEX64(0x1000, ph->align);
    TEST_ASSERT_EQUAL(ELF_PF_FLAGS_R | ELF_PF_FLAGS_W, ph->flags);

}

