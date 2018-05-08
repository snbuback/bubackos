ENTRY(boot_hal)

__ADDR_KERNEL_BASE = 1M;

/* Use nm -nS ./kernel.bin to ensure the __ADDR_KERNEL_START and __ADDR_KERNEL_END points to the limits of the kernel */

SECTIONS {
	. = __ADDR_KERNEL_BASE;
	__ADDR_KERNEL_START = ABSOLUTE(.);

	/* align at 4 KB and load at 1 MB */
	.text ALIGN(4K):
	{
		__ADDR_CODE_START = .;

		*(.multiboot2)
		*(.text)
		*(.rodata)
		*(.js)

		__ADDR_CODE_END = .;
	}

	/* all orphan sections comes here: https://sourceware.org/binutils/docs/ld/Location-Counter.html#Location-Counter */
	. = .;

	.data ALIGN(4K):
	{
		__ADDR_DATA_START = .;
		
		*(.data)
		*(COMMON)
		*(.bss)

		__ADDR_DATA_END = .;
		__ADDR_KERNEL_END = ABSOLUTE(.);
	}

}
