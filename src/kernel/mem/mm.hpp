#pragma once
#include <algo/option.hpp>
#include <algo/range.hpp>
#include <algo/result.hpp>
#include <mem/pm.hpp>

namespace mm {

    /**
     * \brief Describes what kind of memory a certain part of RAM is
     */
    enum PhysicalRegionType {
        /// Memory is free for use
        T_FREE = 0,
        /// Memory is related to ACPI
        T_ACPI,
        /// Memory is reserved and must not be used
        T_RESERVED
    };

    /**
     * \brief Describes a contiguous chunk of RAM
     */
    struct PhysicalMemoryRegion {
        /// Range of the chunk
        range<uintptr_t> reg;
        /// Type of memory the chunk refers to
        PhysicalRegionType type;
    };

    /**
     * \brief Flags that control memory block allocation
     */
    enum AllocFlagsType: uint32_t {
        /// Memory is writable
        AF_RW   = 1 << 0,
        /// Memory is userland-accessible
        AF_USER = 1 << 1
    };

    /// Address type for the target
    using AddressType = uintptr_t;

    /**
     * \brief Allocates a contiguous block of virtual memory and binds physical pages to it
     * \param p           - Page directory
     * \param pageCount   - Number of 2MiB pages to allocate
     * \param flags       - mm::AllocFlagsType
     * \return some(addr) - if allocation succeeded,\n
     *         none()     - if allocation failed
     */
    option<AddressType> alloc(pm::Pml4 *p, size_t pageCount, AllocFlagsType flags);

    /**
     * \brief Frees a block of memory and releases bound physical pages
     * \param p           - Page directory
     * \param start       - Address of the beginning of the block
     * \param pageCount   - Number of 2MiB pages to free
     */
    void free(pm::Pml4 *p, AddressType start, size_t pageCount);

    /**
     * \brief Initializes memory manager settings
     */
    void init();

}
