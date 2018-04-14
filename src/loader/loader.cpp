#include <stdint.h>
#include <mem/paging32.hpp>
#include <sys/debug.hpp>

static pm32::Pagedir m_mainPagedir alignas(0x1000);

extern "C" void loader_main(void) {
    m_mainPagedir.map(0, 0, pm32::PD_PS);
    m_mainPagedir.apply();
    pm32::enable(pm32::PM_ENABLE | pm32::PM_PSE);

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}
