#pragma once
#include <sys/types.h>

extern "C" {
    void *memcpy(void *dst, const void *src, size_t lim);
    void *memmove(void *dst, const void *src, size_t lim);
    void *memset(void *block, int value, size_t lim);
}
