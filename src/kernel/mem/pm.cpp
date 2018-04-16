#include "pm.hpp"
#include <algo/range.hpp>
#include <sys/debug.hpp>
#include <algo/memory.hpp>

#define KERNEL_LOCATION 0x400000
#define PM_TRACKING_INDEX(addr) (static_cast<uintptr_t>(addr) >> 18)
#define PM_TRACKING_BIT(addr)   (static_cast<uintptr_t>(1) << ((static_cast<uintptr_t>(addr) >> 12) & 0x3F))

static range<uintptr_t> m_pagingStructureRange;
static pm::TrackingFieldType *m_pagingTrackingStructure;

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
    uintptr_t trackingAddr = KERNEL_LOCATION - trackingDelta - trackingOld;

    debug::printf("Expanded paging structure region: 0x%lx - 0x%lx, tracking structure change: %luB (was %luB)\n",
        m_pagingStructureRange.start,
        KERNEL_LOCATION,
        trackingDelta,
        trackingOld);
    debug::printf("Tracking data relocated to 0x%lx\n", trackingAddr);

    // 2. prepare new tracking block
    m_pagingTrackingStructure = reinterpret_cast<pm::TrackingFieldType *>(trackingAddr);
    memset(m_pagingTrackingStructure, 0, trackingOld + trackingDelta);
    debug::printf("Clearing %luB\n", trackingOld + trackingDelta);

    // 3. copy old info from loader
    uint32_t *oldTracking32 = reinterpret_cast<uint32_t *>(oldTracking);
    for (uint32_t oldAddr = 0; oldAddr < (trackingOld / 4) << 17; oldAddr += 0x1000) {
        uint32_t idx = oldAddr >> 17;
        uint32_t bit = 1 << ((oldAddr >> 12) & 0x1F);
        
        if (oldTracking32[idx] & bit) {
            debug::printf("Retaining 0x%x\n", oldAddr + m_pagingStructureRange.start);
            setAlloc(static_cast<uint64_t>(oldAddr + m_pagingStructureRange.start));
        }
    }

    // 4. dump allocation map
    m_pagingStructureRange.end = trackingAddr & ~0xFFF;
    dumpAlloc();
}
