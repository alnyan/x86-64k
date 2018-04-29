#include "memory.hpp"
#include <sys/debug.hpp>

extern "C" void *memset(void *block, int value, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        static_cast<char *>(block)[i] = value;
    }
    return block;
}

extern "C" void *memcpy(void *dst, const void *src, size_t count) {
    debug::printf("memcpy %a <- %a, %d\n", dst, src, count);
    for (size_t i = 0; i < count; ++i) {
        static_cast<char *>(dst)[i] = static_cast<const char *>(src)[i];
    }
    return dst;
}
