ENTRY(boot)           /* the name of the entry symbol */

kernel_virtual_start = .;

BASE_ADDR = 0;

. = BASE_ADDR + 0x100000;          /* the code should be relocated to 3 GB + 1 MB */

/* these labels get exported to the code files */
kernel_physical_start = . - BASE_ADDR;

SECTIONS {
	/* align at 4 KB and load at 1 MB */
	.text ALIGN (4K) : AT(ADDR(.text)-BASE_ADDR)
	{
			*(.multiboot2)            /* all text sections from all files */
	    *(.text)            /* all text sections from all files */
	}

	/* align at 4 KB and load at 1 MB + . */
	.rodata ALIGN (4K) : AT(ADDR(.rodata)-BASE_ADDR)
	{
	    *(.rodata*)         /* all read-only data sections from all files */
	}

	/* align at 4 KB and load at 1 MB + . */
	.data ALIGN (4K) : AT(ADDR(.data)-BASE_ADDR)
	{
	    *(.data)            /* all data sections from all files */
	}

	/* align at 4 KB and load at 1 MB + . */
	.bss ALIGN (4K) : AT(ADDR(.bss)-BASE_ADDR)
	{
	    *(COMMON)           /* all COMMON sections from all files */
	    *(.bss)             /* all bss sections from all files */
	}
}
kernel_virtual_end = .;
kernel_physical_end = . - BASE_ADDR;
