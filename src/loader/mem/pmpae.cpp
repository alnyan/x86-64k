#include "pmpae.hpp"
#include <sys/panic.hpp>
#include <algo/memory.hpp>

pm::pae::Pdpt::Pdpt() {
    debug::printf("pm::pae()\n");
    memset(m_entries, 0, sizeof(m_entries));
}

void pm::pae::Pdpt::apply() {
    pm::set(this);
}

void pm::pae::Pdpt::map(pm::pae::AddressType vaddr, pm::pae::AddressType paddr, pm::pae::EntryFlagType flags) {
    debug::printf("pm::pae::map %A -> %A\n", vaddr, paddr);
    // TODO: make sure vaddr and paddr are aligned
    uint32_t pdpti = vaddr >> 34;
    uint32_t pdi = (vaddr >> 21) & 0x1FF;
    if (m_entries[pdpti] & 1) {
        PagedirType pd = reinterpret_cast<PagedirType>(m_entries[pdpti] & 0xFFFFF000);

        pd[pdi] = paddr | flags | 0x81;
    } else {
        // Allocate pd
        uintptr_t addr = pm::alloc();
        assert(addr != 0xFFFFFFFF);
        PagedirType pd = reinterpret_cast<PagedirType>(addr);
        
        pd[pdi] = paddr | flags | 0x81;
        m_entries[pdpti] = reinterpret_cast<PagedirEntry>(pd) | 1;
    }
}

void pm::pae::Pdpt::unmap(pm::pae::AddressType vaddr) {
    debug::printf("pm::pae::unmap %A\n", vaddr);
    uint32_t pdpti = vaddr >> 34;
    uint32_t pdi = (vaddr >> 21) & 0x1FF;
    if (m_entries[pdpti] & 1) {
        PagedirType pd = reinterpret_cast<PagedirType>(m_entries[pdpti] & 0xFFFFF000);
        pd[pdi] = 0;
    }
}
