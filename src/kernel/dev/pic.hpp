#pragma once

#include <stdint.h>
#include <dev/io.hpp>

#define PIC1_COMMAND 0x20
#define PIC2_COMMAND 0xa0
#define PIC1_DATA 0x21
#define PIC2_DATA 0xa1
#define PIC_EOI 0x20
#define PIC_READ_ISR 0x0b

#define ICW1_ICW4 0x01
#define ICW1_INIT 0x10
#define ICW4_8086 0x01

#define DO_EOI_IF_NEEDED(interrupt) {\
    if (interrupt == 7 && !(pic_get_isr() & 0x80)) return;\
        if (interrupt == 15 && !(pic_get_isr() & 0x8000)) {\
            io::out<uint8_t>(PIC1_COMMAND, PIC_EOI);\
            return;\
        }\
    pic_eoi(interrupt);\
}

uint16_t pic_get_isr();
void pic_eoi(uint8_t irq);

void pic_remap(uint8_t offs1, uint8_t offs2);
