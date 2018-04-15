#include "std.hpp"

void *operator new(unsigned long a, int b) {
    return reinterpret_cast<void *>(b);
}
