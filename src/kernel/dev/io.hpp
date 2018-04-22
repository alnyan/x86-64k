#pragma once
#include <stdint.h>

namespace io {

    using PortType = uint16_t;

    // obsolete variants
    template<typename T> void out(PortType port, T value) {
        __asm__ __volatile__ ("out %0, %1" : : "a"(value), "Nd"(port));
    }

    template<typename T> T in(PortType port) {
        T value;
        __asm__ __volatile__ ("in %1, %0" : "=a"(value) : "Nd"(port));
        return value;
    }

    // new variants
#define TYPED_OUT(type, typespec) static inline void out##typespec(PortType port, type value) {\
    __asm__ __volatile__ ("out %0, %1" :: "a"(value), "Nd"(port));\
}
#define TYPED_IN(type, typespec) static inline type in##typespec(PortType port) {\
    type value;\
    __asm__ __volatile__ ("in %1, %0" : "=a"(value) :"Nd"(port));\
    return value;\
}
#define TYPED_IO(type, typespec) TYPED_IN(type, typespec) TYPED_OUT(type, typespec)

    TYPED_IO(uint32_t, d)
    TYPED_IO(uint16_t, w)
    TYPED_IO(uint8_t, b)
    
    static inline void wait() {
        __asm__ __volatile__ ( "outb %%al, $0x80" : : "a"(0) );
    }

}
