.section .text

.global gdt_load
.align 16
gdt_load:
	lgdt (%rdi)

	sub $16, %rsp // ljmp is not working in x64
	movq $8, 8(%rsp)
	movabsq $gdt_reload, %rax
	mov %rax, (%rsp)
	lretq
	// ljmp $0x08, $gdt_reload

.global tr_load
.align 16
tr_load:
    movw $0x28, %ax
    ltr %ax
    
.global gdt_reload
gdt_reload:
	movw $0x10, %ax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	ret
