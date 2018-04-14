.set MULTIBOOT2_MAGIC,      0xE85250D6
.set MULTIBOOT2_FLAGS,      0x00000000
.set MULTIBOOT2_LENGTH,     0x00000010
.set MULTIBOOT2_CHECKSUM,   -(MULTIBOOT2_FLAGS + MULTIBOOT2_LENGTH + MULTIBOOT2_MAGIC)

.section .multiboot

.long MULTIBOOT2_MAGIC
.long MULTIBOOT2_FLAGS
.long MULTIBOOT2_LENGTH
.long MULTIBOOT2_CHECKSUM
.long 0x00000000
.long 0x00000008

.section .text

.global multiboot_entry
.align 16
multiboot_entry:
    lgdt (gdtr32)

    jmp loader_main

.align 16
gdt32:
    .quad 0x0000000000000000
    .quad 0x00CF9A000000FFFF
    .quad 0x00CF92000000FFFF
gdtr32:
    .long gdt32
    .word (gdtr32 - gdt32 - 1)