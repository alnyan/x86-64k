ENTRY(kernel_entry)
SECTIONS {
    . = 0x4FF000000000;

    .text BLOCK(4K) : AT(ADDR(.text) - 0x4FF000000000 + 0x400000)
	{
		*(.text)
	}
	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x4FF000000000 + 0x400000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - 0x4FF000000000 + 0x400000)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - 0x4FF000000000 + 0x400000)
	{
		*(COMMON)
		*(.bss)
	}
}
