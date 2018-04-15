.section .text

.global pae_enable
.align 16
pae_enable:
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4
    ret