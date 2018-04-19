#include "memory.hpp"

void *memcpy(void *dst, const void *src, size_t lim) { return memmove(dst, src, lim); }

void *memmove(void *dst, const void *src, size_t lim) {
    if (lim == 0 || dst == src) return dst;

    auto cdst = reinterpret_cast<char *>(dst);
    auto csrc = reinterpret_cast<const char *>(src);

    if (cdst < csrc) {
        for (size_t i = 0; i < lim; ++i) {
            cdst[i] = csrc[i];
        }
    }
    else {
        for (size_t i = 0, p = lim - 1; i < lim; ++i, --p) {
            cdst[p] = csrc[p];
        }
    }

    return dst;
}

void *memset(void *block, int value, size_t lim) {
    for (size_t i = 0; i < lim; ++i) {
        reinterpret_cast<uint8_t *>(block)[i] = static_cast<uint8_t>(value);
    }
    return block;
}
