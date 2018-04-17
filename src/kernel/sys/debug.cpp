#include "debug.hpp"
#include <algo/itoa.hpp>
#include <sys/panic.hpp>

devices::rs232::SerialPort debug::out(0x3F8);

void debug::assertFail(const char *file, int line, const char *msg) {
    printf("%s:%d: %s\n", file, line, msg);
    panic();
}

void debug::puts(const char *s) {
    while (*s) {
        out.putc(*s++);
    }
}

void debug::puts(const char *s, char c, size_t p) {
    size_t l = strlen(s);
    for (size_t i = l; i < p; ++i) {
        out.putc(c);
    }
    for (size_t i = 0; i < l; ++i) {
        out.putc(s[i]);
    }
}

void debug::print(const str &s) {
    size_t l = s.length();
    for (size_t i = 0; i < l; ++i) {
        out.putc(s[i]);
    }
}

void debug::println(const str &s) {
    size_t l = s.length();
    for (size_t i = 0; i < l; ++i) {
        out.putc(s[i]);
    }
    out.putc('\n');
}

void debug::printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    debug::vprintf(fmt, args);
    va_end(args);
}

void debug::vprintf(const char *fmt, va_list args) {
    char c;
    char buf[itoa::bufferSize];
    uint32_t v32;
    uint64_t v64;
    char padChar = -1;
    size_t padCount = 0;
    while ((c = *fmt)) {
        switch (c) {
        case '%':
            c = *++fmt;
            if (c == '0' || c == ' ') {
                padChar = c;
                c = *++fmt;
            }
            while ((c = *fmt) && c >= '0' && c <= '9') {
                padCount *= 10;
                padCount += c - '0';
                ++fmt;
            }
            switch (c) {
            case 's':
                v64 = va_arg(args, uint64_t); // Addresses are 64-bit
                puts(reinterpret_cast<const char *>(v64), padChar, padCount);
                break;
            case 'd':
                v32 = va_arg(args, uint32_t);
                puts(itoa::itoa<10>(buf, v32), padChar, padCount);
                break;
            case 'x':
                v32 = va_arg(args, uint32_t);
                puts(itoa::itoa<16>(buf, v32), padChar, padCount);
                break;
            case 'u':
                v32 = va_arg(args, uint32_t);
                puts(itoa::itoa<10, true>(buf, v32), padChar, padCount);
                break;
            case 'a':
                v32 = va_arg(args, uint32_t);
                puts("0x");
                puts(itoa::itoa<16>(buf, v32), '0', 8);
                break;
            case 'l':
                c = *++fmt;
                switch (c) {
                case 'd':
                    v64 = va_arg(args, uint64_t);
                    puts(itoa::itoa<10>(buf, v64), padChar, padCount);
                    break;
                case 'x':
                    v64 = va_arg(args, uint64_t);
                    puts(itoa::itoa<16>(buf, v64), padChar, padCount);
                    break;
                case 'u':
                    v64 = va_arg(args, uint64_t);
                    puts(itoa::itoa<10, true>(buf, v64), padChar, padCount);
                    break;
                case 'a':
                    v64 = va_arg(args, uint64_t);
                    puts("0x");
                    puts(itoa::itoa<16>(buf, v64), '0', 16);
                    break;
                default:
                    break;
                }
                break;
            default:
                out.putc('%');
                out.putc(c);
                break;
            }
            break;
        default:
            out.putc(c);
            break;
        }
        ++fmt;
    }
}

