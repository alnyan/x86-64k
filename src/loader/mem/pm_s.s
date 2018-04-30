.section .text

.global pm_enable
.align 16
pm_enable:
    movl %cr0, %eax
    orl $0x80000000, %eax
    movl %eax, %cr0
    ret

.global pm_disable
.align 16
pm_disable:
    movl %cr0, %eax
    andl $~0x80000000, %eax
    movl %eax, %cr0
    ret

.global pse_enable
.align 16
pse_enable:
    movl %cr4, %eax
    orl $0x00000010, %eax
    movl %eax, %cr4
    ret

.global pse_disable
.align 16
pse_disable:
    movl %cr4, %eax
    andl $~0x00000010, %eax
    movl %eax, %cr4
    ret

.global pm_load
.align 16
pm_load:
    movl 4(%esp), %eax
    movl %eax, %cr3
    ret
    
.global pae_enable
.align 16
pae_enable:
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4
ret