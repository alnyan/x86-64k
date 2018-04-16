#include "memory.hpp"

void *memcpy(void *dst, const void *src, size_t lim) {
    for (size_t i = 0; i < lim; ++i) {
        reinterpret_cast<char *>(dst)[i] = reinterpret_cast<const char *>(src)[i];
    }
    return dst;
}

void *memset(void *block, int value, size_t lim) {
    for (size_t i = 0; i < lim; ++i) {
        reinterpret_cast<uint8_t *>(block)[i] = static_cast<uint8_t>(value);
    }
    return block;
}
