#include "debug.hpp"
#include <sys/io.hpp>
#include <algo/itoa.hpp>
#include <sys/panic.hpp>

debug::SerialDebug debug::out(0x3F8);

debug::SerialDebug::SerialDebug(uint16_t port): m_port{port} {
    io::out<uint8_t>(port + 1, 0x00);    // Disable all interrupts
    io::out<uint8_t>(port + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    io::out<uint8_t>(port + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    io::out<uint8_t>(port + 1, 0x00);    //                  (hi byte)
    io::out<uint8_t>(port + 3, 0x03);    // 8 bits, no parity, one stop bit
    io::out<uint8_t>(port + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    io::out<uint8_t>(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void debug::SerialDebug::putc(char c) {
    while (!(io::in<uint8_t>(m_port + 5) & 0x20));
    io::out(m_port, c);
}

void debug::assertFail(const char *file, int line, const char *msg) {
    printf("%s:%d: %s\n", file, line, msg);
    panic();
}

void debug::puts(const char *s) {
    while (*s) {
        out.putc(*s++);
    }
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
        switch (c) {
            case '%':
                switch (c = *++fmt; c) {
                    case 'a':
                        puts("0x");
                        [[fallthrough]];
                    case 'x':
                        v32 = va_arg(args, uint32_t);
                        puts(itoa::itoa<16>(buf, v32));
                        break;
                    case 'd':
                        v32 = va_arg(args, uint32_t);
                        puts(itoa::itoa<10>(buf, v32));
                        break;
                    case 'A':
                        puts("0x");
                        [[fallthrough]];
                    case 'X':
                        v64 = va_arg(args, uint64_t);
                        puts(itoa::itoa<16>(buf, v64));
                        break;
                    case 's':
                        v32 = va_arg(args, uint32_t);
                        puts(reinterpret_cast<const char *>(v32));
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

void debug::dump(const void *buf, size_t lim) {
    const char *cs = reinterpret_cast<const char *>(buf);
    for (size_t b = 0; b < lim; ++b) {
        debug::printf("+%d: %a (%d)\n", b, cs[b], cs[b]);
    }
}
