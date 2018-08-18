OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

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
	.text ALIGN(4K) : 
	{
		*(.text)
	} :text

	.rodata ALIGN(4K) : 
	{
		*(.rodata)
	} :rodata

	.interp :
	{
		*(.interp)
	} :interp :rodata

	.data ALIGN(4K) : 
	{
		*(.data)
	} :data

	.bss ALIGN(4K) : 
	{
		*(.bss)
		*(COMMON)
	} :data

	. = .;

}
