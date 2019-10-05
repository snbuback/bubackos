OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

PHDRS
{
	headers PT_PHDR PHDRS ;
	interp PT_INTERP ;
	/*
	TODO I don't know why but I'm having problem when module running in user mode
	doesn't have all permissions in all sections.
	 */
	text PT_LOAD FILEHDR PHDRS FLAGS (7);
	rodata PT_LOAD FLAGS (7);
	data PT_LOAD FLAGS (7);
}

BASE_ADDR = 4M;

SECTIONS
{

	. = SEGMENT_START("text", BASE_ADDR) + SIZEOF_HEADERS;
	.text ALIGN(4K) : 
	{
		*(.text)
	} :text

	.interp ALIGN(4K) :
	{
		*(.interp)
	} :interp :rodata

/*	.rodata ALIGN(4K) :
	{
		*(.rodata)
	} :rodata
*/
	.data ALIGN(4K) : 
	{
		*(.data)
		*(.rodata)
		/* temporary rodata is in data section, because of a program in
		paging permissions*/
	} :data

	.bss ALIGN(4K) : 
	{
		*(.bss)
		*(COMMON)
	} :data

	. = .;

}
