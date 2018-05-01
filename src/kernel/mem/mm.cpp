#include "mm.hpp"
#include <cstring>

// Hidden implementation details

namespace mm {

    enum VirtualAllocFlagsType: uint32_t {
        VF_RW   = 1 << 0,
        VF_USER = 1 << 1
    };

    enum PhysicalAllocFlagsType: uint32_t {
        PF_RW   = 1 << 0,
        PF_USER = 1 << 1
    };

    using PhysicalPageTrackingType = uint64_t;
    using PhysicalPageType = uintptr_t;

}

// FIXME: only 4GiB
#define PHYS_SIZE 0x1000000000

static range<uintptr_t> m_virtualAllocRange(0x80000000, 0xFFFF00000000);
static mm::PhysicalPageTrackingType m_physicalTrackingStructure[PHYS_SIZE >> 24];

option<mm::AddressType> findVirtualRange(pml4_arc_t<ptse_allocator_t> *p, size_t pageCount, mm::VirtualAllocFlagsType flags) {
    range<uintptr_t> r;
    for (auto rs: m_virtualAllocRange.iter(0x200000)) {
        bool res = false;
        r = range<uintptr_t>(rs, rs + pageCount * 0x200000);

        // Make sure every page is free
        for (auto addr: r.iter(0x200000)) {
            if (p->getMapping(addr, false)) {
                res = true;
                break;
            }
        }

        if (!res) {
            debug::dprintf(" = %la - %la\n", rs, rs + pageCount * 0x200000);
            return rs;
        }
    }
    return option<mm::AddressType>::none();
}

void freeVirtualRange(pml4_arc_t<ptse_allocator_t> *p, mm::AddressType start, size_t pageCount, mm::VirtualAllocFlagsType flags);

result allocPhysicalPages(mm::PhysicalPageType *pages, size_t count, mm::PhysicalAllocFlagsType flags) {
    debug::dprintf("mm::allocPhysicalPages %lu\n", count);
    size_t allocd = 0;
    mm::AddressType last = 0x600000;

    for (mm::AddressType addr = last; addr < PHYS_SIZE; addr += 0x200000) {
        if (allocd == count) {
            break;
        }

        size_t idx = addr >> 27;
        size_t bit = 1 << ((addr >> 21) & 0x3F);

        if (!(m_physicalTrackingStructure[idx] & bit)) {
            debug::dprintf(" = %la\n", addr);
            m_physicalTrackingStructure[idx] |= bit;
            pages[allocd++] = addr;
            last = addr;
            continue;
        }
    }

    if (allocd != count) {
        debug::dprintf("Physical alloc failed, reverting alloc state\n");
        for (size_t i = 0; i < allocd; ++i) {
            mm::AddressType addr = pages[i];
            size_t idx = addr >> 27;
            size_t bit = (addr >> 21) & 0x3F;

            debug::dprintf(" - %la\n", addr);
            m_physicalTrackingStructure[idx] &= ~bit;
        }
        return result(1);
    }

    return result(0);
}

void freePhysicalPages(mm::PhysicalPageType *pages, size_t count, mm::PhysicalAllocFlagsType flags);

void mm::init() {
    range<uintptr_t> x(0x80000000, 0xFFFF00000000);
    m_virtualAllocRange = x;
    memset(m_physicalTrackingStructure, 0, sizeof(m_physicalTrackingStructure));
}

option<mm::AddressType> mm::alloc(pml4_arc_t<ptse_allocator_t> *p, size_t pageCount, mm::AllocFlagsType flags) {
    if (pageCount == 0) {
        return option<mm::AddressType>::none();
    }

    // TODO: this code is assuming flag layouts are identical
    mm::VirtualAllocFlagsType vaflags = mm::VirtualAllocFlagsType(flags);
    mm::PhysicalAllocFlagsType paflags = mm::PhysicalAllocFlagsType(flags);
    page_struct_flags_t pmflags = page_struct_flags_t(flags);

    debug::dprintf("mm::alloc %lu pages\n", pageCount);
    // Make sure we don't allocate huge buffer on stack
    dassert(pageCount <= 32);

    // Find virtual range
    // TODO: convert flags
    auto vr = findVirtualRange(p, pageCount, vaflags);
    if (!vr) {
        debug::dprintf(" = failed to find virtual pages\n");
        return option<mm::AddressType>::none();
    }

    mm::AddressType vstart = *vr;

    // Allocate physical pages
    // TODO: ???
    mm::PhysicalPageType physPages[pageCount];
    auto pr = allocPhysicalPages(physPages, pageCount, paflags);
    if (!pr) {
        debug::dprintf(" = failed to alloc physical pages\n");
        return option<mm::AddressType>::none();
    }

    // Bind physical pages
    for (size_t i = 0; i < pageCount; ++i) {
        mm::AddressType vaddr = vstart + i * 0x200000;
        mm::AddressType paddr = physPages[i];

        p->map(vaddr, paddr, pt_page_size_t::SIZE_2M, pt_page_flags_t(pmflags));
    }

    return option<mm::AddressType>::some(vstart);
}

void mm::free(pml4_arc_t<ptse_allocator_t> *p, mm::AddressType start, size_t count) {
    debug::dprintf("mm::free %la - %la\n", start, start + count * 0x200000);

    for (size_t i = 0; i < count; ++i) {
        mm::AddressType vaddr = start + i * 0x200000;
        
        if (!p->getMapping(vaddr, false)) {
            debug::dpanic("Possible double-free detected: tried to free non-allocated page\n");
        }

        p->unmap(vaddr);
    }
}
