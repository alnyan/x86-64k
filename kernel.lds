ENTRY(kernel_entry)
SECTIONS {
    . = 0x800000000;
  
	.init : {
		*(.init)
	}

    .text :
	{
		*(.multiboot)
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
