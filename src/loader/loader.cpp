#include <stdint.h>
#include <sys/std.hpp>
#include <mem/pm.hpp>
#include <mem/pmpae.hpp>
#include <sys/debug.hpp>
#include <sys/types.h>

extern "C" void loader_main(void) {
    // Map loader here
    pm::setAlloc(0x100000); // Because pdpt
    pm::pae::Pdpt *pdp = new (0x100000) pm::pae::Pdpt;
    pdp->map(0x00000000, 0x00000000, 1 << 7);
    pdp->map(0x00200000, 0x00200000, 1 << 7);

    pdp->apply();
    pae_enable();
    pm_enable();

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}
