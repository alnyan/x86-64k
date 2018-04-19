#pragma once
#include <stdint.h>

namespace io {

    using PortType = uint16_t;

    template<typename T> void out(PortType port, T value) {
        __asm__ __volatile__ ("out %0, %1" : : "a"(value), "Nd"(port));
    }

    template<typename T> T in(PortType port) {
        T value;
        __asm__ __volatile__ ("in %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

}
