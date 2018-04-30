#include "string.hpp"

size_t strlen(const char *s) {
    size_t r = 0;
    while (*s++) {
        ++r;
    }
    return r;
}