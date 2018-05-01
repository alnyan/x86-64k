#pragma once
#include <sys/debug.hpp>

namespace debug {

    class SerialDebug {
    public:
        SerialDebug(uint16_t port);

        void putc(char c);

    private:
        uint16_t m_port;
    };

    extern SerialDebug out;
}
