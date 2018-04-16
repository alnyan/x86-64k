#pragma once
#include <stdint.h>

namespace devices::rs232 {

    using PortType = uint16_t;

    class SerialPort {
    public:
        SerialPort(PortType port);

        void putc(char c);

    private:
        const PortType m_port;
    };

}
