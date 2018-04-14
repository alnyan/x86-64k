#include "panic.hpp"

void panic() {
    debug::printf(" !!! PANIC !!!\n");
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}