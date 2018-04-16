#include "rs232.hpp"
#include <dev/io.hpp>

devices::rs232::SerialPort::SerialPort(devices::rs232::PortType port)
    : m_port{port} {
        
}

void devices::rs232::SerialPort::putc(char c) {
    io::out(m_port, c);
}
