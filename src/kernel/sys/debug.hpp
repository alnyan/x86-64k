#pragma once
#include <stdint.h> 
#include <stdarg.h>
#include <sys/types.h>
#include <dev/rs232.hpp>
#include <algo/string.hpp>

#define assert_stringify(x) #x
#define assert_stringify2(x) assert_stringify(x)
#define assert(v) if (!(v)) { debug::assertFail(__FILE__, __LINE__, "Assertion failed: " assert_stringify2(v)); }

namespace debug {

    extern devices::rs232::SerialPort out;

    void puts(const char *s);
    void puts(const char *s, char padc, size_t pad);
    void print(const str &s);
    void println(const str &s);
    void printf(const char *msg, ...);
    void vprintf(const char *msg, va_list args);

    void dump(const void *buf, size_t lim);

    [[noreturn]] void assertFail(const char *file, int line, const char *msg);

}
