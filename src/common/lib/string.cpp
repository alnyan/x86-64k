#ifndef NO_STDCXX
#include <cstddef>
#include <cstdint>
#else
#include <stddef.h>
#include <stdint.h>
#endif

extern "C" {
    int strcmp(const char *a, const char *b) {
        do {
            if (*a != *b) { return static_cast<int>(*a) - *b; }
            if (*a == 0) return 0;
            a++;
            b++;
        } while (true);
    }

    int strncmp(const char *a, const char *b, size_t num) {
        for (size_t i = 0; i < num; i++) {
            if (*a != *b) { return static_cast<int>(*a) - *b; }
            if (*a == 0) break;
            a++;
            b++;
        }
        return 0;
    }

    int strcoll(const char *a, const char *b) { return strcmp(a, b); }

    size_t strxfrm(char *destination, const char *source, size_t num) {
        for (size_t i = 0; i < num; i++) {
            destination[i] = source[i];
            if (!source[i]) return i;
        }
        return num;
    }

    int memcmp(const void *a, const void *b, size_t num) {
        const char *ca = static_cast<const char*>(a),
            *cb = static_cast<const char*>(b);
        for (size_t i = 0; i < num; i++) {
            if (*ca != *cb) { return static_cast<int>(*ca) - *cb; }
            ca++;
            cb++;
        }
        return 0;
    }

    size_t strlen(const char *s) {
        size_t l = 0;
        while (*s++) { ++l; }
        return l;
    }

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

    void *memcpy(void *dst, const void *src, size_t lim) { return memmove(dst, src, lim); }

    void *memset(void *block, int value, size_t lim) {
        auto blk = reinterpret_cast<uint8_t *>(block);
        for (size_t i = 0; i < lim; ++i) { blk[i] = value; }
        return block;
    }

    void *memchr(void *block, int value, size_t lim) {
        uint8_t ch = value;
        auto blk = reinterpret_cast<uint8_t *>(block);
        for (size_t i = 0; i < lim; ++i, blk++) { 
            if (*blk == ch) return blk;
        }
        return nullptr;
    }

    const char *strerror(int errnum) { return "UNKNOWN_ERROR"; }
}