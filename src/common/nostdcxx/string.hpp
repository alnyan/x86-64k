#pragma once
#include <stddef.h>

extern "C" {
    int strcmp(const char *a, const char *b);
    int strncmp(const char *a, const char *b, size_t num);
    size_t strlen(const char *s);
    int strcoll(const char *s1, const char *s2);
    size_t strxfrm(char *destination, const char *source, size_t num);
    const char *strerror(int errnum);
    void *memcpy(void *dst, const void *src, size_t num);
    void *memmove(void *dst, const void *src, size_t num);
    void *memset(void *block, int value, size_t num);
    const void *memchr(const void *ptr, int value, size_t num);
}