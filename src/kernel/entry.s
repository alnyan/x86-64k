.section .text

.global kernel_entry
.align 16
kernel_entry:
    cli

    call kernel_preinit
    call kernel_main

1:
    hlt
    jmp 1b