.global test_handler_ring3_s
.align 16
test_handler_ring3_s:
	mov %rax, %ss:-8(%rsp)
	mov $0x10, %rax
	mov %ax, %ss

	push %rax
	push %rbx
	push %rcx
	push %rdx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	call test_handler_ring3
	
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rdx
	pop %rcx
	pop %rbx
	pop %rax
	iretq
