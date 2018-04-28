ENTRY(kernel_entry)
SECTIONS {
    . = 0x4000000000;

    .text :
	{
		*(.text)
	}
	.rodata ALIGN(4K) :
	{
		*(.rodata)
	}

	.data ALIGN(4K) :
	{
		*(.data)
	}

	.bss ALIGN(4K) :
	{
		*(COMMON)
		*(.bss)
	}
}
