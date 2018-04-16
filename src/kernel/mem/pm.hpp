#pragma once
#include <stdint.h>

namespace pm {

    using AddressType = uint64_t;
    using FlagsType = uint64_t;

    using PagedirEntry = uint64_t;
    using PdptEntry = uint64_t;
    using Pml4Entry = uint64_t;

    class Pml4 {
    public:
        Pml4();

        void map(AddressType vaddr, AddressType paddr, FlagsType flags);
        void apply();

    private:
        Pml4Entry m_entries[512];
    };

}
