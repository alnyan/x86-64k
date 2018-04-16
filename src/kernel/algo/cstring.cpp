#include "string.hpp"

size_t strlen(const char *s) {
    size_t r = 0;
    while (*s++) {
        ++r;
    }
    return r;
}

const char *strncmp(const char *a, const char *b, size_t lim) {
    size_t i = 0;
    while (i < lim && (*a || *b)) {
        if (*a != *b) {
            return a;
        }
        ++a;
        ++b;
        ++i;
    }
    return nullptr;
}
