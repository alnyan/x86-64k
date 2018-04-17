#include "mm.hpp"
#include <algo/memory.hpp>

using PhysicalPageTrackingType = uint64_t;

// FIXME: only 4GiB
#define PHYS_SIZE 0x1000000000

static const range<pm::AddressType> m_virtualAllocRange(0x80000000, 0xFFFF00000000);
static PhysicalPageTrackingType m_physicalTrackingStructure[PHYS_SIZE >> 24];

option<mm::AddressType> findVirtualRange(pm::Pml4 *p, size_t pageCount, mm::VirtualAllocFlagsType flags) {
    range<pm::AddressType> r;
    for (auto rs: m_virtualAllocRange.iter(0x200000)) {
        bool res = false;
        r = range<pm::AddressType>(rs, rs + pageCount * 0x200000);

        // Make sure every page is free
        for (auto addr: r.iter(0x200000)) {
            if (p->get(addr)) {
                res = true;
                break;
            }
        }

        if (!res) {
            debug::printf(" = %la - %la\n", rs, rs + pageCount * 0x200000);
            return rs;
        }
    }
    return option<mm::AddressType>::none();
}

void freeVirtualRange(pm::Pml4 *p, mm::AddressType start, size_t pageCount, mm::VirtualAllocFlagsType flags);

result allocPhysicalPages(mm::PhysicalPageType *pages, size_t count, mm::PhysicalAllocFlagsType flags) {
    debug::printf("mm::allocPhysicalPages %lu\n", count);
    size_t allocd = 0;
    mm::AddressType last = 0x600000;

    for (mm::AddressType addr = last; addr < PHYS_SIZE; addr += 0x200000) {
        if (allocd == count) {
            break;
        }

        size_t idx = addr >> 27;
        size_t bit = 1 << ((addr >> 21) & 0x3F);

        if (!(m_physicalTrackingStructure[idx] & bit)) {
            debug::printf(" = %la\n", addr);
            m_physicalTrackingStructure[idx] |= bit;
            pages[allocd++] = addr;
            last = addr;
            continue;
        }
    }

    if (allocd != count) {
        debug::printf("Physical alloc failed, reverting alloc state\n");
        for (size_t i = 0; i < allocd; ++i) {
            mm::AddressType addr = pages[i];
            size_t idx = addr >> 27;
            size_t bit = (addr >> 21) & 0x3F;

            debug::printf(" - %la\n", addr);
            m_physicalTrackingStructure[idx] &= ~bit;
        }
        return result(1);
    }

    return result(0);
}

void freePhysicalPages(mm::PhysicalPageType *pages, size_t count, mm::PhysicalAllocFlagsType flags);

void mm::init() {
    memset(m_physicalTrackingStructure, 0, sizeof(m_physicalTrackingStructure));
}

option<mm::AddressType> mm::alloc(pm::Pml4 *p, size_t pageCount, mm::AllocFlagsType flags) {
    if (pageCount == 0) {
        return option<mm::AddressType>::none();
    }

    // TODO: this code is assuming flag layouts are identical
    mm::VirtualAllocFlagsType vaflags = mm::VirtualAllocFlagsType(flags);
    mm::PhysicalAllocFlagsType paflags = mm::PhysicalAllocFlagsType(flags);
    pm::FlagsType pmflags = pm::FlagsType(flags);

    debug::printf("mm::alloc %lu pages\n", pageCount);
    // Make sure we don't allocate huge buffer on stack
    assert(pageCount <= 32);

    // Find virtual range
    // TODO: convert flags
    auto vr = findVirtualRange(p, pageCount, vaflags);
    if (!vr) {
        debug::printf(" = failed to find virtual pages\n");
        return option<mm::AddressType>::none();
    }

    mm::AddressType vstart = *vr;

    // Allocate physical pages
    // TODO: ???
    mm::PhysicalPageType physPages[pageCount];
    auto pr = allocPhysicalPages(physPages, pageCount, paflags);
    if (!pr) {
        debug::printf(" = failed to alloc physical pages\n");
        return option<mm::AddressType>::none();
    }

    // Bind physical pages
    for (size_t i = 0; i < pageCount; ++i) {
        mm::AddressType vaddr = vstart + i * 0x200000;
        mm::AddressType paddr = physPages[i];

        p->map(vaddr, paddr, pmflags);
    }

    return option<mm::AddressType>::some(vstart);
}

void mm::free(pm::Pml4 *p, mm::AddressType start, size_t count) {
    debug::printf("mm::free %la - %la\n", start, start + count * 0x200000);

    for (size_t i = 0; i < count; ++i) {
        mm::AddressType vaddr = start + i * 0x200000;
        
        if (!p->get(vaddr)) {
            panic_msg("Possible double-free detected: tried to free non-allocated page\n");
        }

        p->unmap(vaddr);
    }
}
