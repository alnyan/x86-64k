#pragma once
#include <stdint.h>
#include <mem/pm.hpp>

extern "C" void pae_enable();

namespace pm::pae {

using PagedirEntry = uint64_t;
using PdptEntry = uint64_t;
using AddressType = uint64_t;
using EntryFlagType = uint32_t;

using PagedirType = PagedirEntry *;

class Pdpt {
public:
    Pdpt();

    void apply();

    void map(AddressType vaddr, AddressType paddr, EntryFlagType flags);
    void unmap(AddressType vaddr);
    AddressType operator [](AddressType vaddr) const;

private:
    PdptEntry m_entries[4];
};

}
