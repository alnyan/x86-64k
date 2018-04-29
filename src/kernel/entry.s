.section .text

.global kernel_entry
.align 16
kernel_entry:
    cli

    pushq %rdi
    call _init // scary because libstdc++ can do anything before kernel initialized

    popq %rdi
    call kernel_main

1:
    hlt
    jmp 1b