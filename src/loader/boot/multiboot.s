.set MB_ALIGN_MODS, 1 << 0
.set MB_MEMINFO,    1 << 1
.set MB_FLAGS,      MB_ALIGN_MODS | MB_MEMINFO
.set MB_MAGIC,      0x1BADB002
.set MB_CHECKSUM,   -(MB_MAGIC + MB_FLAGS)

.section .multiboot

.align 16
.long MB_MAGIC
.long MB_FLAGS
.long MB_CHECKSUM

.section .text

.global multiboot_entry
.align 16
multiboot_entry:
    movl %ebx, mb_info_ptr
    movl $stack_top, %esp

    //lgdt (gdtr32)

    call _init
    call loader_main

.align 16
gdt32:
    .quad 0x0000000000000000
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
gdtr32:
    .long gdt32
    .word (gdtr32 - gdt32 - 1)

.global mb_info_ptr
.align 4
mb_info_ptr:
    .long 0

.section .bss
.align 16
stack_bottom:
    .skip 65536
stack_top:
