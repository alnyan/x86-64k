#pragma once
#include <stdint.h>
#include <sys/std.hpp>
#include <sys/debug.hpp>

extern "C" {
    void pse_enable();
    void pae_enable();
    void pm_disable();
    void pm_enable();
    void pm_load(uintptr_t paddr);
}

namespace pm {

    constexpr static const_range<uintptr_t, 0x100000, 0x200000> paging_region;
    extern uintptr_t vaddr;

    template<typename T> void set(T *p) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        dassert(paging_region.contains(addr)); // Paging region is identity-mapped
        vaddr = addr;
        pm_load(vaddr);                       // No address translation needed
    }

    template<typename T> T *current() {
        return reinterpret_cast<T *>(vaddr);
    }

    bool isFree(uintptr_t addr);
    void setAlloc(uintptr_t addr);

    void *trackingPtr();
    uintptr_t alloc(); // Allocates 0x1000-sized region for paging structures

}
