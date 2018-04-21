#include <dev/pic.hpp>

void pic_eoi(uint8_t irq) {
    if (irq & 8) 
        io::out<uint8_t>(PIC2_COMMAND, PIC_EOI);
    io::out<uint8_t>(PIC1_COMMAND, PIC_EOI);
}

void pic_remap(uint8_t offs1, uint8_t offs2) {
    uint8_t a1 = io::in<uint8_t>(PIC1_DATA);
    uint8_t a2 = io::in<uint8_t>(PIC2_DATA);
    
    io::out<uint8_t>(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io::wait();
    io::out<uint8_t>(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io::wait();
    
    io::out<uint8_t>(PIC1_DATA, offs1);
    io::wait();
    io::out<uint8_t>(PIC2_DATA, offs2);
    io::wait();
    
    io::out<uint8_t>(PIC1_DATA, 4);
    io::wait();
    io::out<uint8_t>(PIC2_DATA, 2);
    io::wait();
    
    io::out<uint8_t>(PIC1_DATA, ICW4_8086);
    io::wait();
    io::out<uint8_t>(PIC2_DATA, ICW4_8086);
    io::wait();
    
    io::out<uint8_t>(PIC1_DATA, a1);
    io::out<uint8_t>(PIC2_DATA, a2);
}

uint16_t pic_get_isr() {
    io::out<uint8_t>(PIC1_COMMAND, PIC_READ_ISR);
    io::out<uint8_t>(PIC2_COMMAND, PIC_READ_ISR);
    uint8_t isr1 = io::in<uint8_t>(PIC1_COMMAND);
    return (io::in<uint8_t>(PIC2_COMMAND) << 8) | isr1;
}