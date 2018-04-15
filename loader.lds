ENTRY(multiboot_entry)
SECTIONS {
    . = 0x200000;

    .text BLOCK(4K) : ALIGN(4K)
	{
		*(.multiboot)
		*(.text)
	}
  
	.init : {
		*(.init)
	}
	
	.rodata BLOCK(4K) : ALIGN(4K)
	{
		*(.rodata)
	}

	.data BLOCK(4K) : ALIGN(4K)
	{
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K)
	{
		*(COMMON)
		*(.bss)
	}
}
