#pragma once
#include <sys/paging/ptse_arc_base.hpp>
#include <sys/panic.hpp>
#include <stddef.h>

template <typename TVirtualPtr>
class pt64_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, 0, arc_last_t> {
protected:
    unsigned m_mappedEntries = 0;
    page_table_entry64_t m_tables[PTSE_ENTRIES64] PTSE_ALIGNED = {};

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
public:
    pt64_arc_t(arc_allocator_t *allocator) : ptse_arc_base_t<TVirtualPtr, uint64_t, 0, arc_last_t>(allocator) {}; 
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    void *getStructuresPtr() override { return m_tables; }
};

template <typename TVirtualPtr>
class pd64_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, PTSE_ENTRIES64, pt64_arc_t<TVirtualPtr>> {
protected:
    unsigned m_mappedEntries = 0;
    page_directory_entry64_t m_tables[PTSE_ENTRIES64] PTSE_ALIGNED = {};

    ptse_arc_mapping_status_t mapStatus(TVirtualPtr vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? m_tables[indexOf(vaddr)].flags.size 
                    ? ptse_arc_mapping_status_t::MAPPED_FLAT
                    : ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
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
public:
    pd64_arc_t(arc_allocator_t *allocator) : ptse_arc_base_t<TVirtualPtr, uint64_t, PTSE_ENTRIES64, pt64_arc_t<TVirtualPtr>>(allocator) {}; 
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    void *getStructuresPtr() override { return m_tables; }
};

#define PDPT_ENTRIES(ptrType) (sizeof(ptrType) == sizeof(uint32_t) ? 4 : PTSE_ENTRIES64)
#define PDPT_ALIGNMENT(ptrType) ALIGNED(sizeof(ptrType) == sizeof(uint32_t) ? PTSE_PDPT32_ALIGNMENT : PTSE_SIZEOF)
#define PDPT_FLAGS_RESERVED(ptrType) (sizeof(ptrType) == sizeof(uint32_t) ? PTSE_FLAG_RING3 | PTSE_FLAG_RW : 0)

template <typename TVirtualPtr>
class pdpt_arc_t : public ptse_arc_base_t<TVirtualPtr, uint64_t, PDPT_ENTRIES(TVirtualPtr), pd64_arc_t<TVirtualPtr>> {
protected:
    unsigned m_mappedEntries = 0;
    pdpt_entry_t m_tables[PDPT_ENTRIES(TVirtualPtr)] PDPT_ALIGNMENT(TVirtualPtr) = {};
    const uint64_t PDPT_FLAGS_RESERVED = PDPT_FLAGS_RESERVED(TVirtualPtr);

    ptse_arc_mapping_status_t mapStatus(TVirtualPtr vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? m_tables[indexOf(vaddr)].flags.size 
                    ? ptse_arc_mapping_status_t::MAPPED_FLAT
                    : ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
    ptse_arc_status_t actuallyMap(TVirtualPtr vaddr, uint64_t paddr, page_struct_flags_t flags, bool flat) override {
        auto ix = indexOf(vaddr);
        if (flat && sizeof(TVirtualPtr) == sizeof(uint32_t)) return ptse_arc_status_t::CANNOT_MAP_FLAT; 
        if (m_tables[ix].flags.present) return ptse_arc_status_t::ALREADY_MAPPED;
        pdpt_entry_t entry;
        entry.address = paddr | (uint64_t(flags) & ~PDPT_FLAGS_RESERVED);
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
public:
    pdpt_arc_t(arc_allocator_t *allocator) : ptse_arc_base_t<TVirtualPtr, uint64_t, PDPT_ENTRIES(TVirtualPtr), pd64_arc_t<TVirtualPtr>>(allocator) {}; 
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    void *getStructuresPtr() override { return m_tables; }
    static const unsigned LEVEL_4K = 2;
    static const unsigned LEVEL_2M = 1;
#if __LP64__
    void apply() { 
        __asm__ __volatile__ (
            "lea %0, %%rax\n"
            "mov %%rax, %%cr3\n" :: "m"(m_tables)
        );
    }
#else
    void apply() { 
        __asm__ __volatile__ (
            "lea %0, %%eax\n"
            "mov %%eax, %%cr3\n" :: "m"(m_tables)
        );
    }
#endif
};

class pml4_arc_t : public ptse_arc_base_t<uint64_t, uint64_t, PTSE_ENTRIES64, pdpt_arc_t<uint64_t>> {
protected:
    unsigned m_mappedEntries = 0;
    pml4_entry_t m_tables[PTSE_ENTRIES64] PTSE_ALIGNED = {};

    ptse_arc_mapping_status_t mapStatus(uint64_t vaddr) override {
        return 
            m_tables[indexOf(vaddr)].flags.present 
                ? ptse_arc_mapping_status_t::MAPPED_NOT_FLAT
                : ptse_arc_mapping_status_t::NOT_MAPPED;
    }
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
public:
    pml4_arc_t(arc_allocator_t *allocator) : ptse_arc_base_t<uint64_t, uint64_t, PTSE_ENTRIES64, pdpt_arc_t<uint64_t>>(allocator) {};
    bool readyToDeallocation() override { return m_mappedEntries == 0; }
    void *getStructuresPtr() override { return m_tables; }
    static const unsigned LEVEL_4K = 3;
    static const unsigned LEVEL_2M = 2;
    static const unsigned LEVEL_1G = 1;
#if __LP64__
    void apply() { 
        __asm__ __volatile__ (
            "lea %0, %%rax\n"
            "mov %%rax, %%cr3\n" :: "m"(m_tables)
        );
    }
#else
    void apply() { 
        __asm__ __volatile__ (
            "lea %0, %%eax\n"
            "mov %%eax, %%cr3\n" :: "m"(m_tables)
        );
    }
#endif
};