#pragma once
#include <sys/paging/ptse_arc_base.hpp>
#include <sys/panic.hpp>
#include <stddef.h>

class pt32_arc_t : public ptse_arc_base_t<uint32_t, uint32_t, 0, arc_last_t> {
protected:
    unsigned m_mappedEntries = 0;
    page_table_entry32_t m_tables[PTSE_ENTRIES32] PTSE_ALIGNED = {};

    bool vaddrBelowLimit(uint32_t vaddr) override { return vaddr < PTSE_PAGE_SIZE * PTSE_ENTRIES32; }
    ptse_arc_status_t actuallyMap(uint32_t vaddr, uint32_t paddr, page_struct_flags_t flags) override {
        auto ix = vaddr / PTSE_PAGE_SIZE;
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        page_table_entry32_t entry;
        entry.address = paddr | uint32_t(flags);
        entry.flags.present = 1;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(uint32_t vaddr) override {
        auto ix = vaddr / PTSE_PAGE_SIZE;
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
public:
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    void *getStructuresPtr() override { return m_tables; }
};

class pd32_arc_t : public ptse_arc_base_t<uint32_t, uint32_t, PTSE_ENTRIES32, pt32_arc_t>  {
protected:
    unsigned m_mappedEntries = 0;
    page_directory_entry32_t m_tables[PTSE_ENTRIES32] PTSE_ALIGNED = {};

    ptse_arc_mapping_status_t mapStatus(uint32_t vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? m_tables[indexOf(vaddr)].flags.size 
                    ? ptse_arc_mapping_status_t::MAPPED_FLAT
                    : ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
    ptse_arc_status_t actuallyMap(uint32_t vaddr, uint32_t paddr, page_struct_flags_t flags, bool flat) override {
        auto ix = indexOf(vaddr);
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        page_directory_entry32_t entry;
        entry.address = paddr | uint32_t(flags);
        entry.flags.present = 1;
        entry.flags.size = flat;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(uint32_t vaddr) override {
        auto ix = indexOf(vaddr);
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
    virtual unsigned indexOf(uint32_t vaddr) { return vaddr / (PTSE_PAGE_SIZE * PTSE_ENTRIES32); }
    virtual uint32_t levelDown(uint32_t vaddr) { return vaddr % (PTSE_PAGE_SIZE * PTSE_ENTRIES32); }
public:
    void *getStructuresPtr() override { return m_tables; }
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
#ifdef __LP32__
    void apply() { 
        __asm__ __volatile__ (
            "lea %0, %%eax\n"
            "mov %%eax, %%cr3\n" :: "m"(m_tables)
        );
    }
#endif
};