#include "memory.hpp"

void *memset(void *block, int value, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        static_cast<char *>(block)[i] = value;
    }
    return block;
}
