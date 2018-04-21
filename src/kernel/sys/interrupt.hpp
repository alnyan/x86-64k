#pragma once
#include <stdint.h>

#define ISR_MAX_INTERRUPTS 256
#define ISR_IRQ_BASE 0x20

typedef struct  __attribute__ ((packed)) {
    uintptr_t eip;
    uintptr_t cs;
    uintptr_t eflags;
    union {
        struct __attribute__ ((packed)) {
            uintptr_t esp;
            uintptr_t ss;
        } userToKernel;
        uintptr_t kernelToKernelStack[0];
    };
} interrupt_frame_t;

typedef void (*interrupt_handler)(interrupt_frame_t *frame);
typedef void (*interrupt_error_handler)(interrupt_frame_t *frame, uint32_t error);

void isr_init(void);
void isr_load_and_unmask(void);

void isr_set_handler(int interrupt, interrupt_handler handler, int rpl);
void isr_set_error_handler(int interrupt, interrupt_error_handler handler);
