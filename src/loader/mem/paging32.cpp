#include "paging32.hpp"
#include <sys/debug.hpp>
#include <sys/panic.hpp>

static pm32::ModeFlagType m_pm32ModeFlags = 0;
pm32::Pagedir *pm32::current, *pm32::kernel;

extern "C" {
    void pm32_enable();
    void pm32_disable();
    void pse_enable();
    void pse_disable();
    void pm32_load(pm32::AddressType paddr);
}

pm32::AddressType pm32::toPhys(pm32::AddressType v) {
    return v;
}

pm32::ModeFlagType pm32::flags() {
    return m_pm32ModeFlags;
}

void pm32::enable(pm32::ModeFlagType flags) {
    m_pm32ModeFlags = flags;
    if (!(flags & PM_ENABLE)) {
        pm32_disable();
    }

    if (flags & PM_PSE) {
        pse_enable();
    } else {
        pse_disable();
    }

    if (flags & PM_ENABLE) {
        pm32_enable();
    }
}

void pm32::set(pm32::Pagedir *pd) {
    assert(!(reinterpret_cast<AddressType>(pd) & 0xFFF));
    current = pd;

    // Address of pm32::* is uint32_t
    // TODO: check for flags (PSE?)
    AddressType phys = toPhys(reinterpret_cast<AddressType>(pd));
    pm32_load(phys);
}

void pm32::invlpgPse(pm32::AddressType page) {
    for (AddressType a = page; a < page + 0x400000; ++a) {
        __asm__ __volatile__ ("invlpg (%0)"::"a"(a));
    }
}

pm32::Pagedir::Pagedir() {
    memset(m_entries, 0, sizeof(m_entries));
}

void pm32::Pagedir::map(pm32::AddressType vaddr, pm32::AddressType paddr, pm32::PdeFlagType flags) {
    debug::printf("pm32::map %a -> %a\n", vaddr, paddr);
    assert(flags & PD_PS);
    assert(!(vaddr & 0xFFF) && !(paddr & 0xFFF));

    uint32_t pdi = vaddr >> 22;

    if (m_entries[pdi] & PD_PRESENT) {
        panic_msg("Mapping already present");
    } else {
        m_entries[pdi] = paddr | flags | PD_PRESENT;
        if (pm32::flags() & PM_ENABLE) {
            pm32::invlpgPse(vaddr);
        }
    }
}

void pm32::Pagedir::apply() {
    pm32::set(this);
}