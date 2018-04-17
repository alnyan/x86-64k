#pragma once
#include <stdint.h>
#include "../loader/loader.hpp"
#include <algo/option.hpp>

namespace pm {

    using AddressType = uint64_t;
    enum FlagsType: uint64_t {
        F_PRESENT = 1 << 0,
        F_RW      = 1 << 1,
        F_PS      = 1 << 7
    };

    using PagedirEntry = uint64_t;
    using PdptEntry = uint64_t;
    using Pml4Entry = uint64_t;

    using TrackingFieldType = uint64_t;
    using RefcountType = uint16_t;

    class Pml4 {
    public:
        Pml4();

        void map(AddressType vaddr, AddressType paddr, FlagsType flags);
        option<uintptr_t> get(AddressType vaddr) const;

        void apply();

    private:
        Pml4Entry m_entries[512];
    };

    void dumpAlloc();
    void free(uintptr_t ptr);
    option<uintptr_t> alloc();
    void setAlloc(uintptr_t addr);

    Pml4 *kernel();
    Pml4 *current();

    RefcountType incRefs(uintptr_t addr);
    RefcountType decRefs(uintptr_t addr);

    void retainLoaderPaging(const LoaderData *loaderData);

}
