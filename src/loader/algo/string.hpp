#pragma once
#include <sys/types.h>

extern "C" {
    const char *strncmp(const char *a, const char *b, size_t l);
    size_t strlen(const char *s);
}
