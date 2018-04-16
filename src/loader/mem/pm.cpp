#include "pm.hpp"
#include <sys/types.h>
#include <algo/memory.hpp>

uintptr_t pm::vaddr;

static uint32_t m_dataTracker[(0x200000 - 0x100000) >> 17]; // FIXME: magic numbers

void *pm::trackingPtr() {
    return m_dataTracker;
}

bool pm::isFree(uintptr_t addr) {
    assert(paging_region.contains(addr));
    assert(!(addr & 0xFFF));
    
    size_t idx = (addr - paging_region.start) >> 17;
    size_t bit = 1 << (((addr - paging_region.start) >> 12) & 0x1F);

    if (!(m_dataTracker[idx] & bit)) {
        return true;
    }

    return false;
}

uintptr_t pm::alloc() {
    debug::printf("pm::alloc\n");
    // Brute force scan here
    for (uintptr_t addr = paging_region.start; addr < paging_region.end; addr += 0x1000) {
        if (isFree(addr)) {        
            setAlloc(addr);
            memset(reinterpret_cast<void *>(addr), 0, 0x1000);
            debug::printf(" = %a\n", addr);
            return addr;
        }
    }

    return 0xFFFFFFFF;
}

void pm::setAlloc(uintptr_t addr) {
    size_t idx = (addr - paging_region.start) >> 17;
    size_t bit = 1 << (((addr - paging_region.start) >> 12) & 0x1F);

    m_dataTracker[idx] |= bit;
}
