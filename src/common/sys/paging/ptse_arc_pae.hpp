#pragma once
#include <sys/paging/ptse_arc_base.hpp>
#include <sys/panic.hpp>
#include <stddef.h>

template <typename TVirtualPtr>
class pt64_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, 0, arc_last_t> {
protected:
    unsigned m_mappedEntries = 0;
    page_table_entry64_t m_tables[PTSE_ENTRIES64] = {};

    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    bool vaddrBelowLimit(TVirtualPtr vaddr) override { return vaddr < PTSE_PAGE_SIZE * PTSE_ENTRIES64; }
    ptse_arc_status_t actuallyMap(TVirtualPtr vaddr, uint64_t paddr, page_struct_flags_t flags) override {
        auto ix = vaddr / PTSE_PAGE_SIZE;
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        page_table_entry64_t entry;
        entry.address = paddr | uint64_t(flags);
        entry.flags.present = 1;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(TVirtualPtr vaddr) override {
        auto ix = vaddr / PTSE_PAGE_SIZE;
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
};

template <typename TVirtualPtr>
class pd64_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, PTSE_ENTRIES64, pt64_arc_t<TVirtualPtr>> {
protected:
    unsigned m_mappedEntries = 0;
    page_directory_entry64_t m_tables[PTSE_ENTRIES64] = {};

    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    ptse_arc_mapping_status_t mapStatus(TVirtualPtr vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? m_tables[indexOf(vaddr)].flags.size 
                    ? ptse_arc_mapping_status_t::MAPPED_FLAT
                    : ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
    bool vaddrBelowLimit(TVirtualPtr vaddr) override { return indexOf(vaddr) < PTSE_ENTRIES64; }
    ptse_arc_status_t actuallyMap(TVirtualPtr vaddr, uint64_t paddr, page_struct_flags_t flags, bool flat) override {
        auto ix = indexOf(vaddr);
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        page_directory_entry64_t entry;
        entry.address = paddr | uint64_t(flags);
        entry.flags.present = 1;
        entry.flags.size = flat;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(TVirtualPtr vaddr) override {
        auto ix = indexOf(vaddr);
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
    virtual unsigned indexOf(TVirtualPtr vaddr) { return vaddr / (PTSE_PAGE_SIZE * PTSE_ENTRIES64); }
    virtual TVirtualPtr levelDown(TVirtualPtr vaddr) { return vaddr % (PTSE_PAGE_SIZE * PTSE_ENTRIES64); }
};

template <typename TVirtualPtr, size_t Entries>
class pdpt_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, Entries, pd64_arc_t<TVirtualPtr>> {
protected:
    unsigned m_mappedEntries = 0;
    pdpt_entry_t m_tables[Entries] = {};

    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    ptse_arc_mapping_status_t mapStatus(TVirtualPtr vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? m_tables[indexOf(vaddr)].flags.size 
                    ? ptse_arc_mapping_status_t::MAPPED_FLAT
                    : ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
    bool vaddrBelowLimit(TVirtualPtr vaddr) override { return indexOf(vaddr) < Entries; }
    ptse_arc_status_t actuallyMap(TVirtualPtr vaddr, uint64_t paddr, page_struct_flags_t flags, bool flat) override {
        auto ix = indexOf(vaddr);
        if (flat && sizeof(TVirtualPtr) == sizeof(uint32_t)) return ptse_arc_status_t::CANNOT_MAP_FLAT; 
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        pdpt_entry_t entry;
        entry.address = paddr | uint64_t(flags);
        entry.flags.present = 1;
        entry.flags.size = flat;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(TVirtualPtr vaddr) override {
        auto ix = indexOf(vaddr);
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
    virtual unsigned indexOf(TVirtualPtr vaddr) { return vaddr / (PTSE_PAGE_SIZE * PTSE_ENTRIES64 * PTSE_ENTRIES64); }
    virtual TVirtualPtr levelDown(TVirtualPtr vaddr) { return vaddr % (PTSE_PAGE_SIZE * PTSE_ENTRIES64 * PTSE_ENTRIES64); }
};

class pml4_arc_t : public ptse_arc_base_t<uint64_t, uint64_t, PTSE_ENTRIES64, pdpt_arc_t<uint64_t, PTSE_ENTRIES64>> {
protected:
    unsigned m_mappedEntries = 0;
    pml4_entry_t m_tables[PTSE_ENTRIES64] = {};

    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    ptse_arc_mapping_status_t mapStatus(uint64_t vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
    bool vaddrBelowLimit(uint64_t vaddr) override { return indexOf(vaddr) < PTSE_ENTRIES64; }
    ptse_arc_status_t actuallyMap(uint64_t vaddr, uint64_t paddr, page_struct_flags_t flags, bool flat) override {
        auto ix = indexOf(vaddr);
        if (flat) return ptse_arc_status_t::CANNOT_MAP_FLAT;
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        pml4_entry_t entry;
        entry.address = paddr | uint64_t(flags);
        entry.flags.present = 1;
        m_tables[ix] = entry;
        m_mappedEntries++;
        return ptse_arc_status_t::OK;
    }
    ptse_arc_status_t actuallyUnmap(uint64_t vaddr) override {
        auto ix = indexOf(vaddr);
        if (!m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_UNMAPPED;
        m_tables[ix].flags.present = 0;
        m_mappedEntries--;
        return ptse_arc_status_t::OK;
    }
    virtual unsigned indexOf(uint64_t vaddr) { return vaddr / (static_cast<uint64_t>(PTSE_PAGE_SIZE) * PTSE_ENTRIES64 * PTSE_ENTRIES64 * PTSE_ENTRIES64); }
    virtual uint64_t levelDown(uint64_t vaddr) { return vaddr % (static_cast<uint64_t>(PTSE_PAGE_SIZE) * PTSE_ENTRIES64 * PTSE_ENTRIES64 * PTSE_ENTRIES64); }
};