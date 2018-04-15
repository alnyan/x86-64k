#include "string.hpp"

const char *strncmp(const char *a, const char *b, size_t lim) {
    size_t off = 0;
    while (off < lim && (*a || *b)) {
        if (*a != *b) {
            return a;
        }
        ++a;
        ++b;
        ++off;
    }
    return 0;
}
