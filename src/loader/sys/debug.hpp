#pragma once
#include <stdint.h> 
#include <sys/std.hpp>
#include <stdarg.h>
#include <sys/types.h>

#define assert_stringify(x) #x
#define assert_stringify2(x) assert_stringify(x)
#define assert(v) if (!(v)) { debug::assertFail(__FILE__, __LINE__, "Assertion failed: " assert_stringify2(v)); }

namespace debug {

    class SerialDebug {
    public:
        SerialDebug(uint16_t port);

        void putc(char c);

    private:
        uint16_t m_port;
    };

    extern SerialDebug out;

    void puts(const char *s);
    void printf(const char *msg, ...);
    void vprintf(const char *msg, va_list args);

    void dump(const void *buf, size_t lim);

    [[noreturn]] void assertFail(const char *file, int line, const char *msg);

}
