#pragma once

#ifndef NO_STDCXX
#include <cstdarg>
#include <cstddef>
#else
#include <stdarg.h>
#include <stddef.h>
#endif

#define dassert(v) if (!(v)) { debug::dpanicf("[%s:%d] Assertion failed: " #v, __FILE__, __LINE__); }
#define dassertf(v, f, ...) if (!(v)) { debug::dpanicf("[%s:%d] " #f, __FILE__, __LINE__, ##__VA_ARGS__); }

namespace debug {
    // local implementation
    void dputchar(char ch);
    [[noreturn]] void dhalt();

    // shared implementation
    [[noreturn]] void dpanicf(const char *format, ...);
    [[noreturn]] void dvpanicf(const char *format, va_list args);
    [[noreturn]] void dpanic(const char *str);

    void dsetinfo(bool enabled);
    bool dgetinfo();

    void dputs(const char *s);
    void dpadputs(const char *s, char p, size_t pl);

    void dprintf(const char *msg, ...);
    void dvprintf(const char *msg, va_list args);
    
    void ddump(const void *buf, size_t lim);
}