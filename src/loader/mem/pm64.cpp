#include "pm64.hpp"
#include <sys/panic.hpp>
#include <algo/memory.hpp>

pm::pm64::Pml4::Pml4() {
    debug::printf("pm::pm64()\n");
    memset(m_entries, 0, sizeof(m_entries));
}

void pm::pm64::Pml4::apply() {
    pm::set(this);
}

void pm::pm64::Pml4::map(pm::pm64::AddressType vaddr, pm::pm64::AddressType paddr, pm::pm64::EntryFlagType flags) {
    debug::printf("pm::pm64::map %A -> %A\n", vaddr, paddr);
    // TODO: make sure vaddr and paddr are aligned
    uint64_t pml4i64 = vaddr >> 39;
    uint64_t pdpi64 = (vaddr >> 30) & 0x1FF;
    uint64_t pdi64 = (vaddr >> 21) & 0x1FF;
    uint32_t pml4i = static_cast<uint32_t>(pml4i64);
    uint32_t pdpi = static_cast<uint32_t>(pdpi64);
    uint32_t pdi = static_cast<uint32_t>(pdi64);

    debug::printf("vaddr: %A, pml4i: %a, pdpi: %a, pdi: %a\n", vaddr, pml4i, pdpi, pdi);

    if (m_entries[pml4i] & 1) {
        debug::printf("  PDP exists\n");
        uint64_t pdp64 = m_entries[pml4i] & 0xFFFFF000;
        PdptType pdp = reinterpret_cast<PdptType>(static_cast<uint32_t>(pdp64));
        debug::printf("pdp64 = %A\n", pdp64);

        if (static_cast<uint32_t>(pdp[pdpi]) & 1) {
            debug::printf("  PD exists\n");
            uint64_t pd64 = pdp[pdpi] & 0xFFFFF000;
            PagedirType pd = reinterpret_cast<PagedirType>(static_cast<uint32_t>(pd64));
            debug::printf("pd64 = %A\n", pd64);

            pd[pdi] = paddr | static_cast<uint64_t>(flags | 0x81);
        } else {
            debug::printf("  Adding PD\n");
            // Allocate pd
            uintptr_t pdaddr = pm::alloc();
            assert(pdaddr != 0xFFFFFFFF);
            PagedirType pd = reinterpret_cast<PagedirType>(pdaddr);

            pd[pdi] = paddr | static_cast<uint64_t>(flags | 0x81);
            pdp[pdpi] = static_cast<uint64_t>(pdaddr | 1);
        }
    } else {
        debug::printf("  Adding PDP\n");
        // Allocate pdp
        uintptr_t pdp_addr = pm::alloc();
        assert(pdp_addr != 0xFFFFFFFF);
        PdptType pdp = reinterpret_cast<PdptType>(pdp_addr);
        uintptr_t pdaddr = pm::alloc();
        assert(pdaddr != 0xFFFFFFFF);
        PagedirType pd = reinterpret_cast<PagedirType>(pdaddr);
        memset(pdp, 0, 0x1000);
        memset(pd, 0, 0x1000);

        debug::printf("pdp32 = %a, pd32 = %a\n", pdp_addr, pdaddr);

        pd[pdi] = paddr | flags | 0x81;
        pdp[pdpi] = static_cast<uint64_t>(pdaddr | 1);
        m_entries[pml4i] = static_cast<uint64_t>(pdp_addr | 1);

        debug::printf("pdp[pdpi]64 = %A\n", pdp[pdpi]);
    }
    // if (m_entries[pdpti] & 1) {
    //     PagedirType pd = reinterpret_cast<PagedirType>(m_entries[pdpti] & 0xFFFFF000);

    //     pd[pdi] = paddr | flags | 0x81;
    // } else {
    //     // Allocate pd
    //     uintptr_t addr = pm::alloc();
    //     assert(addr != 0xFFFFFFFF);
    //     PagedirType pd = reinterpret_cast<PagedirType>(addr);
        
    //     pd[pdi] = paddr | flags | 0x81;
    //     m_entries[pdpti] = reinterpret_cast<PagedirEntry>(pd) | 1;
    // }
}

void pm::pm64::Pml4::unmap(pm::pm64::AddressType vaddr) {
    debug::printf("pm::pm64::unmap %A\n", vaddr);
    while (true);
    // uint32_t pdpti = vaddr >> 34;
    // uint32_t pdi = (vaddr >> 21) & 0x1FF;
    // if (m_entries[pdpti] & 1) {
    //     PagedirType pd = reinterpret_cast<PagedirType>(m_entries[pdpti] & 0xFFFFF000);
    //     pd[pdi] = 0;
    // }
}
