#pragma once
#include <sys/paging/ptables.hpp>
#include <sys/panic.hpp>
#include <stddef.h>

enum class ptse_arc_status_t {
    OK,
    VADDR_ABOVE_LIMIT,
    VADDR_ABOVE_LIMIT_IN_NESTED,
    PAGE_LEVEL_TOO_LARGE,
    ALREADY_MAPPED,
    ALREADY_UNMAPPED,
    CANNOT_MAP_IN_FLAT_ZONE,
    CANNOT_MAP_FLAT
};

enum class ptse_arc_mapping_status_t {
    NOT_MAPPED,
    MAPPED_NOT_FLAT,
    MAPPED_FLAT,
    VADDR_ABOVE_LIMIT
};

struct arc_last_t{};

template <typename TVirtualPointer, typename TPhysicalPointer, size_t NestedCount, typename TNested>
class ptse_arc_base_t {
private:
    TNested *m_innerArcs[NestedCount] = {};
protected:
    virtual bool readyToDeallocation() = 0;
    virtual bool vaddrBelowLimit(TVirtualPointer vaddr) = 0;
    virtual unsigned indexOf(TVirtualPointer vaddr) = 0;
    virtual TVirtualPointer levelDown(TVirtualPointer vaddr) = 0;
    virtual ptse_arc_mapping_status_t mapStatus(TVirtualPointer vaddr) = 0;
    virtual ptse_arc_status_t actuallyMap(TVirtualPointer vaddr, TPhysicalPointer paddr, page_struct_flags_t flags, bool flat) = 0;
    virtual ptse_arc_status_t actuallyUnmap(TVirtualPointer vaddr) = 0;
public:
    ptse_arc_base_t();

    ptse_arc_status_t map(TVirtualPointer vaddr, TPhysicalPointer paddr, unsigned pageLevel, page_struct_flags_t flags) {
        if (!vaddrBelowLimit(vaddr)) return ptse_arc_status_t::VADDR_ABOVE_LIMIT;
        if (pageLevel == 0) { // mapping directly
            return actuallyMap(vaddr, paddr, flags, true);
        }
        else { // going down
            switch (mapStatus(vaddr)) {
                case ptse_arc_mapping_status_t::NOT_MAPPED:
                    auto status = actuallyMap(vaddr, paddr, flags, true);
                    if (status != ptse_arc_status_t::OK) return status;
                    [[fallthrough]]
                case ptse_arc_mapping_status_t::MAPPED_NOT_FLAT:
                    break;
                case ptse_arc_mapping_status_t::VADDR_ABOVE_LIMIT: 
                    return ptse_arc_status_t::VADDR_ABOVE_LIMIT_IN_NESTED; // wtf?
                case ptse_arc_mapping_status_t::MAPPED_FLAT:
                    return ptse_arc_status_t::CANNOT_MAP_IN_FLAT_ZONE;
            }
            auto ix = indexOf(vaddr);
            auto arc = m_innerArcs[ix];
            if (arc == nullptr) { 
                m_innerArcs[ix] = arc = new TNested();
                return arc->map(levelDown(vaddr), paddr, pageLevel - 1, flags);
            }
        }
    }

    ptse_arc_status_t unmap(TVirtualPointer vaddr) {
        if (!vaddrBelowLimit(vaddr)) return ptse_arc_status_t::VADDR_ABOVE_LIMIT;
        auto ix = indexOf(vaddr);
        auto arc = m_innerArcs[ix];
        if (arc == nullptr) { // no inner arc, unmapping directly
            return actuallyUnmap(vaddr);
        }
        else { // going down
            auto status = arc->unmap(levelDown(vaddr));
            if (!status == ptse_arc_status_t::OK) return status;
            if (arc->readyToDeallocation()) {
                m_innerArcs[ix] = nullptr;
                delete arc;
            }
        }
    }
};

template <typename TVirtualPointer, typename TPhysicalPointer, size_t AnyValue>
class ptse_arc_base_t<TVirtualPointer, TPhysicalPointer, AnyValue, arc_last_t> {
protected:
    virtual bool readyToDeallocation() = 0;
    virtual bool vaddrBelowLimit(TVirtualPointer vaddr) = 0;
    virtual ptse_arc_status_t actuallyMap(TVirtualPointer vaddr, TPhysicalPointer paddr, page_struct_flags_t flags) = 0;
    virtual ptse_arc_status_t actuallyUnmap(TVirtualPointer vaddr) = 0;
public:
    ptse_arc_base_t();

    ptse_arc_status_t map(TVirtualPointer vaddr, TPhysicalPointer paddr, unsigned pageLevel, page_struct_flags_t flags) {
        if (!vaddrBelowLimit(vaddr)) return ptse_arc_status_t::VADDR_ABOVE_LIMIT;
        if (pageLevel == 0) {
            return actuallyMap(vaddr, paddr, flags);
        }
        else return ptse_arc_status_t::PAGE_LEVEL_TOO_LARGE;
    }

    ptse_arc_status_t unmap(TVirtualPointer vaddr) {
        if (!vaddrBelowLimit(vaddr)) return ptse_arc_status_t::VADDR_ABOVE_LIMIT;
        return actuallyUnmap(vaddr);
    }
};