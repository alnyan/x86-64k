#pragma once
#include <stdint.h>
#include "../loader/loader.hpp"
#include <algo/option.hpp>
#include <algo/result.hpp>

#define PM_PML4I(vaddr) ((vaddr) >> 39)
#define PM_PDPTI(vaddr) (((vaddr) >> 30) & 0x1FF)
#define PM_PDI(vaddr) (((vaddr) >> 21) & 0x1FF)
#define PM_4K_ALIGN(addr) ((addr) & ~0xFFF)
#define PM_4K_LOWER(addr) ((addr) & 0xFFF)
#define PM_4K_ALIGNED(addr) (!PM_4K_LOWER(addr))
#define PM_2M_ALIGN(addr) (((addr) & ~0x1FFFFF))
#define PM_2M_LOWER(addr) ((addr) & 0x1FFFFF)
#define PM_2M_ALIGNED(addr) (!PM_2M_LOWER(addr))
#define PM_MAKEADDR(pml4i, pdpti, pdi, off) (((pml4i) << 39) | ((pdpti) << 30) | ((pdi) << 21) | (off))

namespace pm {

    /// Address type for the target
    using AddressType = uint64_t;

    /**
     * \brief Flags describing page kind
     */
    enum FlagsType: uint64_t {
        /// Page is present
        F_PRESENT = 1 << 0,
        /// Page is writable
        F_RW      = 1 << 1,
        /// Page is accessible from userland
        F_USER    = 1 << 2,
        /// Page is large
        F_PS      = 1 << 7
    };

    /// Entry of page directory
    using PagedirEntry = uint64_t;
    /// Entry of PDPT
    using PdptEntry = uint64_t;
    /// Entry of PML4
    using Pml4Entry = uint64_t;

    /// Bit field for allocation tracking
    using TrackingFieldType = uint64_t;
    /// Used to count references on a paging structure
    using RefcountType = uint16_t;

    /**
     * \brief Main paging class on x86-64: PML4 - top level of paging hierarchy
     * 2MiB pages are used
     */
    class Pml4 {
    public:
        /**
         * \brief Initializes empty PML4
         */
        Pml4();

        /**
         * \brief Adds vaddr->paddr 2MiB page mapping to the PML4
         * \param vaddr     - Virtual address
         * \param paddr     - Physical address
         * \param flags     - pm::FlagsType
         */
        void map(AddressType vaddr, AddressType paddr, FlagsType flags);

        /**
         * \brief Unmaps 2MiB page from the PML4
         * \param vaddr     - Virtual address
         * \return result(0) - on success\n
         *         result(1) - if entry related to vaddr was not found
         */
        result unmap(AddressType vaddr);

        /**
         * \brief Obtains physical address of a 2MiB page from virtual one
         * \param vaddr     - Virtual address
         * \return some(paddr) - physical address of a page\n
         *         none()   - if related page was not found
         */
        option<uintptr_t> get(AddressType vaddr) const;

        /**
         * \brief Makes the PML4 current
         */
        void apply();

        /**
         * \brief Dumps list of mappings in the PML4
         */
        void dump();

    private:
        Pml4Entry m_entries[512];
    };

    /**
     * \brief Dumps paging structure allocation status
     */
    void dumpAlloc();
    
    /**
     * \brief Deallocates paging structure
     */
    void free(uintptr_t ptr);

    /**
     * \brief Allocates a 4KiB block for paging structure
     * \return some(addr)   - if allocation succeeded,\n
     *         none()       - if allocation failed
     */
    option<uintptr_t> alloc();

    /**
     * \brief Marks paging structure as allocated
     * \param addr          - Address of a paging structure
     */
    void setAlloc(uintptr_t addr);

    /**
     * \return Kernel PML4
     */
    Pml4 *kernel();
    
    /**
     * \return Currently used PML4
     */
    Pml4 *current();

    /**
     * \brief Increases refcount on a paging structure
     * \param addr          - Address of a paging structure
     * \return Increased reference count
     */
    RefcountType incRefs(uintptr_t addr);

    /**
     * \brief Decreases refcount (if non-zero) on a paging structure
     * \param addr          - Address of a paging structure
     * \return Decreases reference count
     */
    RefcountType decRefs(uintptr_t addr);

    /**
     * \brief Retrieves paging info from loader stage
     * \param loaderData    - Pointer to loader data structure
     */
    void retainLoaderPaging(const LoaderData *loaderData);

}
