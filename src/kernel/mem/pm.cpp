#include "pm.hpp"
#include <algo/range.hpp>
#include <sys/debug.hpp>
#include <algo/memory.hpp>
#include <sys/panic.hpp>

#define KERNEL_LOCATION 0x400000
#define PM_TRACKING_INDEX(addr) (static_cast<uintptr_t>(addr) >> 18)
#define PM_TRACKING_BIT(addr)   (static_cast<uintptr_t>(1) << ((static_cast<uintptr_t>(addr) >> 12) & 0x3F))
#define PM_REFCOUNT_INDEX(addr) (static_cast<uintptr_t>(addr) >> 12)

static range<uintptr_t> m_pagingStructureRange;
static pm::TrackingFieldType *m_pagingTrackingStructure;
static pm::RefcountType *m_pagingRefcountStructure;
static size_t m_pagingRefcountSize;

static pm::Pml4 *m_kernel;
static pm::Pml4 *m_current;

pm::Pml4::Pml4() {
    memset(m_entries, 0, sizeof(m_entries));
}

void pm::Pml4::map(pm::AddressType vaddr, pm::AddressType paddr, pm::FlagsType flags) {
    debug::printf("pm::map(0x%lx) 0x%lx -> 0x%lx\n", this, vaddr, paddr);
    assert(!(vaddr & 0x1FFFFF));
    assert(!(paddr & 0x1FFFFF));

    size_t pml4i = vaddr >> 39;
    size_t pdpti = (vaddr >> 30) & 0x1FF;
    size_t pdi = (vaddr >> 21) & 0x1FF;
    
    if (m_entries[pml4i] & pm::FlagsType::F_PRESENT) {
        Pml4Entry pml4e = m_entries[pml4i];
        PdptEntry *pdpt = reinterpret_cast<PdptEntry *>(pml4e & ~0xFFF);

        if (pdpt[pdpti] & pm::FlagsType::F_PRESENT) {
            PdptEntry pdpte = pdpt[pdpti];
            PagedirEntry *pd = reinterpret_cast<PagedirEntry *>(pdpte & ~0xFFF);

            if (pd[pdi] & pm::FlagsType::F_PRESENT) {
                panic_msg("Entry is already present\n");
            }

            pd[pdi] = paddr | flags | pm::FlagsType::F_PRESENT | pm::FlagsType::F_PS;
            incRefs(reinterpret_cast<uintptr_t>(pd));
        } else {
            // Allocate new PD
            uintptr_t pdaddr = *alloc();

            PagedirEntry *pd = reinterpret_cast<PagedirEntry *>(pdaddr);

            pd[pdi] = paddr | flags | pm::FlagsType::F_PRESENT | pm::FlagsType::F_PS;
            pdpt[pdpti] = pdaddr | pm::FlagsType::F_PRESENT | pm::FlagsType::F_RW;
            incRefs(pdaddr);
            incRefs(reinterpret_cast<uintptr_t>(pdpt));
        }
    } else {
        // Allocate PDPT and PD
        uintptr_t pdptaddr = *alloc();
        uintptr_t pdaddr = *alloc();

        PdptEntry *pdpt = reinterpret_cast<PdptEntry *>(pdptaddr);
        PagedirEntry *pd = reinterpret_cast<PagedirEntry *>(pdaddr);

        pd[pdi] = paddr | flags | pm::FlagsType::F_PRESENT | pm::FlagsType::F_PS;
        pdpt[pdpti] = pdaddr | pm::FlagsType::F_PRESENT | pm::FlagsType::F_RW;
        m_entries[pml4i] = pdptaddr | pm::FlagsType::F_PRESENT | pm::FlagsType::F_RW;
        incRefs(pdaddr);
        incRefs(pdptaddr);
        incRefs(reinterpret_cast<uintptr_t>(m_entries));
    }
}

option<uintptr_t> pm::Pml4::get(pm::AddressType vaddrFull) const {
    debug::printf("pm::get(0x%lx) 0x%lx\n", this, vaddrFull);
    uintptr_t vaddr = vaddrFull & ~0x1FFFFF;
    uintptr_t low = vaddrFull & 0x1FFFFF;

    size_t pml4i = vaddr >> 39;
    size_t pdpti = (vaddr >> 30) & 0x1FF;
    size_t pdi = (vaddr >> 21) & 0x1FF;

    if (m_entries[pml4i] & pm::FlagsType::F_PRESENT) {
        Pml4Entry pml4e = m_entries[pml4i];
        PdptEntry *pdpt = reinterpret_cast<PdptEntry *>(pml4e & ~0xFFF);

        if (pdpt[pdpti] & pm::FlagsType::F_PRESENT) {
            PdptEntry pdpte = pdpt[pdpti];
            PagedirEntry *pd = reinterpret_cast<PagedirEntry *>(pdpte & ~0xFFF);

            if (pd[pdi] & pm::FlagsType::F_PRESENT) {
                return option<uintptr_t>((pd[pdi] & ~0x1FFFFF) | low);
            }
        }
    }

    return option<uintptr_t>::none();
}

void pm::dumpAlloc() {
    debug::printf("Dumping tracking info:\n");
    for (uintptr_t i = m_pagingStructureRange.start; i < m_pagingStructureRange.end; i += 0x1000) {
        uintptr_t idx = PM_TRACKING_INDEX(i - m_pagingStructureRange.start);
        uintptr_t bit = PM_TRACKING_BIT(i - m_pagingStructureRange.start);

        if (m_pagingTrackingStructure[idx] & bit) {
            debug::printf(" * (%ld:%ld) 0x%lx\n", idx, ((i - m_pagingStructureRange.start) >> 12) & 0x3F, i);
        }
    }
}

void pm::setAlloc(uintptr_t addr) {
    assert(m_pagingStructureRange.contains(addr));
    assert(!(addr & 0xFFF));

    uintptr_t idx = PM_TRACKING_INDEX(addr - m_pagingStructureRange.start);
    uintptr_t bit = PM_TRACKING_BIT(addr - m_pagingStructureRange.end);

    m_pagingTrackingStructure[idx] |= bit;
}

option<uintptr_t> pm::alloc() {
    debug::printf("pm::alloc()\n");

    for (uintptr_t addr = m_pagingStructureRange.start; addr < m_pagingStructureRange.end; addr += 0x1000) {
        uintptr_t idx = PM_TRACKING_INDEX(addr - m_pagingStructureRange.start);
        uintptr_t bit = PM_TRACKING_BIT(addr - m_pagingStructureRange.end);

        if (!(m_pagingTrackingStructure[idx] & bit)) {
            m_pagingTrackingStructure[idx] |= bit;
            debug::printf(" = 0x%lx\n", addr);
            return option<uintptr_t>::some(addr);
        }
    }

    return option<uintptr_t>::none();
}

pm::Pml4 *pm::kernel() {
    return m_kernel;
}

pm::Pml4 *pm::current() {
    return m_current;
}

pm::RefcountType pm::incRefs(uintptr_t addr) {
    debug::printf("pm::++ 0x%lx\n", addr);

    assert(m_pagingStructureRange.contains(addr));
    assert(!(addr & 0xFFF));

    uintptr_t idx = PM_REFCOUNT_INDEX(addr - m_pagingStructureRange.start);

    return ++m_pagingRefcountStructure[idx];
}

pm::RefcountType pm::decRefs(uintptr_t addr) {
    debug::printf("pm::-- 0x%lx\n", addr);

    assert(m_pagingStructureRange.contains(addr));
    assert(!(addr & 0xFFF));

    uintptr_t idx = PM_REFCOUNT_INDEX(addr - m_pagingStructureRange.start);

    if (m_pagingRefcountStructure[idx]) {
        return --m_pagingRefcountStructure[idx];
    }

    return 0;
}

void pm::retainLoaderPaging(const LoaderData *loaderData) {
    m_pagingStructureRange.start = loaderData->loaderPagingBase;
    m_pagingStructureRange.end = loaderData->loaderPagingBase + loaderData->loaderPagingSize;
    pm::TrackingFieldType *oldTracking = reinterpret_cast<pm::TrackingFieldType *>(loaderData->loaderPagingTracking);

    debug::printf("Retained paging data from loader: 0x%lx - 0x%lx (%lu entries), tracking_ptr = 0x%lx\n",
        m_pagingStructureRange.start,
        m_pagingStructureRange.end,
        (m_pagingStructureRange.end - m_pagingStructureRange.start) / 0x1000,
        oldTracking);

    // TODO: this is assuming loader is mapped at 0x200000 and kernel is at 0x400000
    // 1. expand paging data to 0x400000 and relocate addresses
    uintptr_t expandDelta = KERNEL_LOCATION - m_pagingStructureRange.end;
    uintptr_t trackingDelta = (expandDelta >> 18) * 8;
    uintptr_t trackingOld = ((m_pagingStructureRange.end - m_pagingStructureRange.start) >> 18) * 8;
    uintptr_t trackingAddr = (KERNEL_LOCATION - trackingDelta - trackingOld) & ~0xFFF;
    size_t refcountSize = ((KERNEL_LOCATION - m_pagingStructureRange.start) / 0x1000) * sizeof(pm::RefcountType);
    uintptr_t refcountAddr = trackingAddr + trackingDelta + trackingOld;

    debug::printf("Expanded paging structure region: 0x%lx - 0x%lx, tracking structure change: %luB + %luB for refcounting (was %luB)\n",
        m_pagingStructureRange.start,
        KERNEL_LOCATION,
        trackingDelta,
        refcountSize,
        trackingOld);
    
    debug::printf("Refcounting data allocated at 0x%lx\n", refcountAddr);
    debug::printf("Tracking data relocated to 0x%lx\n", trackingAddr);

    // 2. prepare new tracking block
    m_pagingTrackingStructure = reinterpret_cast<pm::TrackingFieldType *>(trackingAddr);
    m_pagingRefcountStructure = reinterpret_cast<pm::RefcountType *>(refcountAddr);
    m_pagingRefcountSize = refcountSize;
    memset(m_pagingTrackingStructure, 0, trackingOld + trackingDelta);
    memset(m_pagingRefcountStructure, 0, m_pagingRefcountSize);
    debug::printf("Clearing %luB\n", trackingOld + trackingDelta);

    // 3. copy old info from loader and setup refcounting
    uint32_t *oldTracking32 = reinterpret_cast<uint32_t *>(oldTracking);
    for (uint32_t oldAddr = 0; oldAddr < (trackingOld / 4) << 17; oldAddr += 0x1000) {
        uint32_t idx = oldAddr >> 17;
        uint32_t bit = 1 << ((oldAddr >> 12) & 0x1F);
        
        if (oldTracking32[idx] & bit) {
            debug::printf("Retaining 0x%x\n", oldAddr + m_pagingStructureRange.start);
            setAlloc(static_cast<uint64_t>(oldAddr + m_pagingStructureRange.start));

            // Iterate page directory and fill refcount
            uint64_t *s = reinterpret_cast<uint64_t *>(oldAddr + m_pagingStructureRange.start);
            for (size_t i = 0; i < 512; ++i) {
                uint64_t entry = s[i];
                if (entry & 1) {
                    incRefs(reinterpret_cast<uintptr_t>(s));
                }
            }
        }
    }

    // 4. dump allocation map
    m_pagingStructureRange.end = trackingAddr & ~0xFFF;
    dumpAlloc();

    // 5. set current paging structure
    m_kernel = reinterpret_cast<Pml4 *>(m_pagingStructureRange.start);
    m_current = m_kernel;  
}