#include <sys/debug.hpp>

#ifndef NO_STDCXX
#include <cstdint>
#include <cstring>
#else
#include <stdint.h>
#include <nostdcxx/string.hpp>
#endif

// это очень временно
namespace itoa {
    constexpr size_t bufferSize = sizeof(unsigned) * 8 + 1;
    constexpr char chars[] = "0123456789abcdef";

    template<size_t base, bool u = false> char *itoa(char *buf, unsigned val) {
        static_assert(base != 0 && base <= 16);
        if (!val) {
            buf[0] = '0';
            buf[1] = 0;
            return buf;
        }

        bool sign;
        size_t n = 0, r, j;

        if constexpr (base == 10 && !u) {
            if (static_cast<int>(val) < 0) {
                sign = true;
                val = static_cast<unsigned>(-static_cast<int>(val));
            } else {
                sign = false;
            }
        }

        while (val) {
            buf[n++] = chars[val % base];
            val /= base;
        }

        if constexpr (base == 10 && !u) {
            if (sign) {
                buf[n] = '-';
                r = n + 1;
            } else {
                r = n--;
            }
        } else {
            r = n--;
        }

        j = 0;
        while (n > j) {
            buf[n] ^= buf[j];
            buf[j] ^= buf[n];
            buf[n] ^= buf[j];
            --n;
            ++j;
        }

        buf[r] = 0;

        return buf;
    }
}

void debug::dputs(const char *s) {
    while (*s) {
        dputchar(*s++);
    }
}

void debug::dpadputs(const char *s, char c, size_t p) {
    size_t l = strlen(s);
    for (size_t i = l; i < p; ++i) {
        dputchar(c);
    }
    for (size_t i = 0; i < l; ++i) {
        dputchar(s[i]);
    }
}

void debug::dpanic(const char *s) {
    dpanicf(s);
    dhalt();
}

void debug::dpanicf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dvpanicf(fmt, args);
    va_end(args);

    dhalt();
}

void debug::dvpanicf(const char *s, va_list args) {
    dputs("[panic]: ");
    dvprintf(s, args);

    dhalt();
}

void debug::dprintf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    dvprintf(fmt, args);
    va_end(args);
}

void debug::dvprintf(const char *fmt, va_list args) {
    char c;
    char buf[itoa::bufferSize];
    uint32_t v32;
    uint64_t v64;
    while ((c = *fmt)) {
        char padChar = -1;
        size_t padCount = 0;
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
                v64 = va_arg(args, uintptr_t);
                dpadputs(reinterpret_cast<const char *>(v64), padChar, padCount);
                break;
            case 'd':
                v32 = va_arg(args, uint32_t);
                dpadputs(itoa::itoa<10>(buf, v32), padChar, padCount);
                break;
            case 'x':
                v32 = va_arg(args, uint32_t);
                dpadputs(itoa::itoa<16>(buf, v32), padChar, padCount);
                break;
            case 'u':
                v32 = va_arg(args, uint32_t);
                dpadputs(itoa::itoa<10, true>(buf, v32), padChar, padCount);
                break;
            case 'a':
                v32 = va_arg(args, uint32_t);
                dputs("0x");
                dpadputs(itoa::itoa<16>(buf, v32), '0', 8);
                break;
            case 'l':
                c = *++fmt;
                switch (c) {
                case 'd':
                    v64 = va_arg(args, uint64_t);
                    dpadputs(itoa::itoa<10>(buf, v64), padChar, padCount);
                    break;
                case 'x':
                    v64 = va_arg(args, uint64_t);
                    dpadputs(itoa::itoa<16>(buf, v64), padChar, padCount);
                    break;
                case 'u':
                    v64 = va_arg(args, uint64_t);
                    dpadputs(itoa::itoa<10, true>(buf, v64), padChar, padCount);
                    break;
                case 'a':
                    v64 = va_arg(args, uint64_t);
                    dputs("0x");
                    dpadputs(itoa::itoa<16>(buf, v64), '0', 16);
                    break;
                default:
                    break;
                }
                break;
            default:
                dputchar('%');
                dputchar(c);
                break;
            }
            break;
        default:
            dputchar(c);
            break;
        }
        ++fmt;
    }
}