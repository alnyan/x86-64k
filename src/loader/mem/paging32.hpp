#pragma once
#include <stdint.h>
#include <algo/memory.hpp>

namespace pm32 {

using PagedirEntry = uint32_t;
using PdeFlagType = uint32_t;
using ModeFlagType = uint32_t;
using AddressType = uint32_t;

constexpr PdeFlagType PD_PRESENT = 1 << 0;
constexpr PdeFlagType PD_PS      = 1 << 7;

constexpr ModeFlagType PM_ENABLE = 1 << 0;
constexpr ModeFlagType PM_PSE    = 1 << 1;

class Pagedir {
public:
    Pagedir();

    void map(AddressType vaddr, AddressType paddr, PdeFlagType flags);
    void apply();

private:
    PagedirEntry m_entries[1024];
};

extern Pagedir *current;
extern Pagedir *kernel;

// FIXME: this is assuming identity mapped loader
void invlpgPse(AddressType page);
AddressType toPhys(AddressType v);
void enable(ModeFlagType flags);
ModeFlagType flags();
void set(Pagedir *pd);

}