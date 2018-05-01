#include <sys/io.hpp>
#include <algo/itoa.hpp>
#include <sys/lodebug.hpp>

debug::SerialDebug debug::out(0x3F8);

debug::SerialDebug::SerialDebug(uint16_t port): m_port{port} {
    io::out<uint8_t>(port + 1, 0x00);    // Disable all interrupts
    io::out<uint8_t>(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    io::out<uint8_t>(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    io::out<uint8_t>(port + 1, 0x00);    //                  (hi byte)
    io::out<uint8_t>(port + 3, 0x03);    // 8 bits, no parity, one stop bit
    io::out<uint8_t>(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    io::out<uint8_t>(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void debug::SerialDebug::putc(char c) {
    while (!(io::in<uint8_t>(m_port + 5) & 0x20));
    io::out(m_port, c);
    io::out(0xe9, c);
}

void debug::dputchar(char ch) {
    out.putc(ch);
}

void debug::dhalt() {
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}