#pragma once
#include <stdint.h>
#include <algo/memory.hpp>
#include <sys/debug.hpp>
#include <sys/panic.hpp>

namespace pm32 {

using PagedirEntry = uint32_t;
using PdeFlagType = uint32_t;
using AddressType = uint32_t;

constexpr PdeFlagType PD_PRESENT =  1 << 0;
constexpr PdeFlagType PD_PS =       1 << 7;

class Pagedir {
public:
    Pagedir() {
        memset(m_entries, 0, sizeof(m_entries));
    }

    void map(AddressType vaddr, AddressType paddr, PdeFlagType flags) {
        debug::printf("pm32::map %a -> %a\n", vaddr, paddr);
        assert(flags & PD_PS);
        uint32_t pdi = vaddr >> 22;

        if (m_entries[pdi] & PD_PRESENT) {
            panic_msg("Mapping already present");
        }
    }

private:
    PagedirEntry m_entries[1024];
};

}