#include "debug.hpp"
#include <algo/itoa.hpp>
#include <cstring>
#include <dev/io.hpp>
#include <sys/panic.hpp>

#define BOCHS_CON_PORT 0xe9

devices::CharDevice* s_debug_devs[debug::MAX_DEBUG_DEVICES];

static void broadcast(char ch) {
    for (unsigned i = 0; i < debug::MAX_DEBUG_DEVICES; i++) {
        if (s_debug_devs[i] != nullptr) s_debug_devs[i]->putchar(ch);
    }
#ifndef NO_BOCHS_BROADCAST
    io::outb(BOCHS_CON_PORT, ch);
#endif
}

void debug::init() {
    memset(s_debug_devs, 0, sizeof(s_debug_devs));
}

void debug::regOutDev(devices::CharDevice *dev) {
    for (unsigned i = 0; i < debug::MAX_DEBUG_DEVICES; i++) {
        if (s_debug_devs[i] == nullptr) {
            s_debug_devs[i] = dev;
            return;
        }
    }
}

void debug::assertFail(const char *file, int line, const char *msg) {
    printf("%s:%d: %s\n", file, line, msg);
    panic();
}

void debug::puts(const char *s) {
    while (*s) {
        broadcast(*s++);
    }
}

void debug::puts(const char *s, char c, size_t p) {
    size_t l = strlen(s);
    for (size_t i = l; i < p; ++i) {
        broadcast(c);
    }
    for (size_t i = 0; i < l; ++i) {
        broadcast(s[i]);
    }
}

void debug::print(const str &s) {
    size_t l = s.length();
    for (size_t i = 0; i < l; ++i) {
        broadcast(s[i]);
    }
}

void debug::println(const str &s) {
    size_t l = s.length();
    for (size_t i = 0; i < l; ++i) {
        broadcast(s[i]);
    }
    broadcast('\n');
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
                broadcast('%');
                broadcast(c);
                break;
            }
            break;
        default:
            broadcast(c);
            break;
        }
        ++fmt;
    }
}

