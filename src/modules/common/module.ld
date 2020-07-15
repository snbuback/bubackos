OUTPUT_FORMAT("elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(_start)

STACK_SIZE = 4K;
BASE_ADDR = 4M;

MEMORY
{
  mem_code      (rx)  : ORIGIN = BASE_ADDR, LENGTH = 1M
  /* TODO remove execute permission from data */
  mem_data      (rwx) : ORIGIN = 8M, LENGTH = 1M
  mem_stack     (rwx) : ORIGIN = 9M, LENGTH = 1M
}

SECTIONS
{
    PROVIDE (__executable_start = SEGMENT_START("text-segment", BASE_ADDR)); . = SEGMENT_START("text-segment", BASE_ADDR) + SIZEOF_HEADERS;
    .interp         : { *(.interp) }
    .gnu.version    : { *(.gnu.version) }
    .gnu.version_d  : { *(.gnu.version_d) }
    .gnu.version_r  : { *(.gnu.version_r) }

    /** All code segments **/
    /* Initialisers */
    .init           :
    {
        KEEP (*(SORT_NONE(.init)))
    }

    /* Code */
	.text : ALIGN(4K)
	{
        *(.text*)
	}

    /* Finalisers */
    .fini           :
    {
        KEEP (*(SORT_NONE(.fini)))
    }
    PROVIDE (etext = .);


    .rodata         : { *(.rodata .rodata.* .gnu.linkonce.r.*) }

    . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
    .eh_frame       : { KEEP (*(.eh_frame)) *(.eh_frame.*) }
    .gnu_extab      : { *(.gnu_extab) }
    .gcc_except_table   : { *(.gcc_except_table .gcc_except_table.*) }
    .tdata          :
    {
        PROVIDE_HIDDEN (__tdata_start = .);
        *(.tdata .tdata.* .gnu.linkonce.td.*)
    }
    .tbss           : { *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon) }
    .preinit_array    :
    {
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP (*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
    }
    .init_array    :
    {
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
        KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
        PROVIDE_HIDDEN (__init_array_end = .);
    }
    .fini_array    :
    {
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
        KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
        PROVIDE_HIDDEN (__fini_array_end = .);
    }

    .data           :
    {
        *(.data .data.* .gnu.linkonce.d.*)
        SORT(CONSTRUCTORS)
    }
    .data1          : { *(.data1) }
    _edata = .; PROVIDE (edata = .);
    . = .;
    __bss_start = .;
    .bss            :
    {
        *(.dynbss)
        *(.bss .bss.* .gnu.linkonce.b.*)
        *(COMMON)
    }
    . = ALIGN(32 / 8);
    _end = .; PROVIDE (end = .);
    . = DATA_SEGMENT_END (.);
}
