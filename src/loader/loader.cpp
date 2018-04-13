#include <stdint.h>

extern "C" void loader_main(void) {
    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}