OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(module_init)

PHDRS
{
	headers PT_PHDR PHDRS ;
	interp PT_INTERP ;
	text PT_LOAD FILEHDR PHDRS ;
	rodata PT_LOAD  ;
	data PT_LOAD ;
}

BASE_ADDR = 4M;

SECTIONS
{

	. = SEGMENT_START("text", BASE_ADDR) + SIZEOF_HEADERS;
	.text :
	{
		*(.text)
	} :text

	.interp :
	{
		*(.interp)
	} :interp :rodata

	.rodata :
	{
		*(.rodata)
	} :rodata

	.data :
	{
		*(.data)
	} :data

	.bss :
	{
		*(.bss)
		*(COMMON)
	} :data

	. = .;

}
