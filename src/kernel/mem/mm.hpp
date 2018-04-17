#pragma once
#include <algo/option.hpp>
#include <algo/range.hpp>
#include <algo/result.hpp>
#include <mem/pm.hpp>

namespace mm {

    enum PhysicalRegionType {
        T_FREE = 0,
        T_ACPI,
        T_RESERVED
    };

    struct PhysicalMemoryRegion {
        range<uintptr_t> reg;
        PhysicalRegionType type;
    };

    enum VirtualAllocFlagsType: uint32_t {
    };

    enum PhysicalAllocFlagsType: uint32_t {
    };

    enum AllocFlagsType: uint32_t {
    };

    using PhysicalPageType = uintptr_t;
    using AddressType = uintptr_t;

    option<AddressType> alloc(pm::Pml4 *p, size_t pageCount, AllocFlagsType flags);
    void free(pm::Pml4 *p, AddressType start, size_t pageCount, AllocFlagsType flags);

    void init();

}
