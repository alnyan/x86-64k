#include "rs232.hpp"
#include <dev/io.hpp>

void devices::rs232::SerialPort::putchar(char c) {
    while (!(io::in<uint8_t>(m_port + 5) & 0x20));
    io::out(m_port, c);
}

char devices::rs232::SerialPort::getchar() {
    return '\0';
}