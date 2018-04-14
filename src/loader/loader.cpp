#include <stdint.h>
#include <mem/paging32.hpp>
#include <sys/debug.hpp>

static pm32::Pagedir m_mainPagedir alignas(0x1000);

extern "C" void loader_main(void) {

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}
