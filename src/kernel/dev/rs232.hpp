#pragma once
#include <dev/chardev.hpp>
#include <stdint.h>

namespace devices::rs232 {

    using PortType = uint16_t;

    class SerialPort : public devices::CharDevice {
    public:
        SerialPort(PortType port) : m_port(port) {};

        void putchar(char c) override;
        char getchar() override;

    private:
        const PortType m_port;
    };

}
