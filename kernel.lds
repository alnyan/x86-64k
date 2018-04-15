ENTRY(kernel_entry)
SECTIONS {
    . = 0x800000000;
  
	.init : {
		*(.init)
	}

    .text : AT(ADDR(.text) - 0x800000000 + 0x800000)
	{
		*(.multiboot)
		*(.text)
	}

	.rodata ALIGN(4K) : AT(ADDR(.rodata) - 0x800000000 + 0x800000)
	{
		*(.rodata)
	}

	.data ALIGN(4K) : AT(ADDR(.data) - 0x800000000 + 0x800000)
	{
		*(.data)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss) - 0x800000000 + 0x800000)
	{
		*(COMMON)
		*(.bss)
	}
}
