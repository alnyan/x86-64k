#pragma once
#include <stdint.h>
#include <mem/pm.hpp>

namespace pm::pm64 {

using PagedirEntry = uint64_t;
using PdptEntry = uint64_t;
using Pml4Entry = uint64_t;
using AddressType = uint64_t;
using EntryFlagType = uint32_t;

using PdptType = PdptEntry *;
using PagedirType = PagedirEntry *;

class Pml4 {
public:
    Pml4();

    void apply();

    void map(AddressType vaddr, AddressType paddr, EntryFlagType flags);
    void unmap(AddressType vaddr);
    AddressType operator [](AddressType vaddr) const;

private:
    Pml4Entry m_entries[512];
};

}
