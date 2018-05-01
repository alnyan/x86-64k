#pragma once
#include <sys/paging/ptables.hpp>
#include <stddef.h>
#include <opt/option.hpp>
#include <opt/status.hpp>
#include <sys/debug.hpp>

enum class pt_page_size_t {
    SIZE_NONE,
    SIZE_4K,
    SIZE_4M, // used only in i386 PSE
    SIZE_2M = SIZE_4M,
    SIZE_1G,
    SIZE_LARGE
};

enum pt_page_flags_t {
    PAGE_FLAG_RW,
    PAGE_FLAG_RING3,
};

enum class pt_arc_status_t {
    OK,
    VADDR_ABOVE_LIMIT,
    VADDR_ABOVE_LIMIT_IN_NESTED,
    PAGE_SIZE_INVALID,
    ALREADY_MAPPED,
    ALREADY_UNMAPPED,
    HAS_ARC_THERE,
    CANNOT_MAP_IN_FLAT_ZONE,
    CANNOT_MAP_FLAT,
    PHYSICAL_PTR_IS_NOT_ALIGNED,
    VIRTUAL_PTR_IS_NOT_ALIGNED,
    ALLOCATED_ARC_IS_NOT_ALIGNED,
    MAPPED_BUT_NO_ARC,
    NOT_IMPLEMENTED,
    CANNOT_APPLY_NON_ROOT_ARC
};

class arc_allocator_t {
public:
    virtual void *allocate() = 0;
    virtual void deallocate(void *ptr) = 0;
};

#define PT_ARC_MAP(index, paddr) { m_children[index] = paddr; m_mappedCount++; }
#define PT_ARC_UNMAP(index) { m_children[index] = 0; m_mappedCount--; }

template <typename TVirtPtr, typename TPhysPtr>
class arc_last_t {
public:
    void *m_children;
    static const TVirtPtr REQ_PALIGN = 0;
    size_t m_mappedCount;
    status<pt_arc_status_t> map(TVirtPtr vaddr, TPhysPtr paddr, pt_page_size_t psz, pt_page_flags_t flags) 
    { return STATUS_AUTO_ERR(pt_arc_status_t::NOT_IMPLEMENTED); }

    status<pt_arc_status_t> unmap(TVirtPtr vaddr)
    { return STATUS_AUTO_ERR(pt_arc_status_t::NOT_IMPLEMENTED); }

    option<TPhysPtr> getMapping(TVirtPtr vaddr, bool adjustPtr)
    { return option<TPhysPtr>::none(); }
};

// todo: type traits
template <typename TAllocator, bool IsRoot,
    pt_page_size_t CurrentLevel,
    typename TVirtPtr, typename TPhysPtr, 
    bool CanChildrenHaveArcs, size_t ChildrenCount,
    typename TChildArc, bool CanMapFlat,
    TVirtPtr ReqValign, TPhysPtr ReqPalign,
    int IndexShift, TVirtPtr IndexMask,
    typename TFlagTranslator>
class pt_arc_t {
    template <typename A, bool B,
        pt_page_size_t C,
        typename TVirtPtr2, typename TPhysPtr2, 
        bool D, size_t E,
        typename F, bool G,
        TVirtPtr2 H, TPhysPtr2 I,
        int J, TVirtPtr2 K,
        typename L>
    friend class pt_arc_t;
    static const TVirtPtr REQ_PALIGN = ReqPalign;
protected:
    TVirtPtr m_children[ChildrenCount] ALIGNED(PTSE_SIZEOF) = {};
    TVirtPtr translateFlags(pt_page_flags_t flags, bool flat) {
        TFlagTranslator lator;
        return lator.translateFlags(flags, flat);
    }
private:
    TChildArc *m_childArcs[CanChildrenHaveArcs ? ChildrenCount : 0] = {};
    TAllocator m_allocator;
    size_t m_mappedCount;
public:
    virtual ~pt_arc_t() {
        for (auto arc : m_childArcs) {
            if (arc != nullptr) {
                arc->~TChildArc();
                m_allocator.deallocate(arc);
            }
        }
    }
    status<pt_arc_status_t> map(TVirtPtr vaddr, TPhysPtr paddr, pt_page_size_t psz, pt_page_flags_t flags) {
        auto index = (vaddr & IndexMask) >> IndexShift;

        if (psz == CurrentLevel) {
            if (!CanMapFlat) return STATUS_AUTO_ERR(pt_arc_status_t::CANNOT_MAP_FLAT);
            if (CanChildrenHaveArcs && m_childArcs[index]) return STATUS_AUTO_ERR(pt_arc_status_t::HAS_ARC_THERE);

            if (vaddr & ReqValign) return STATUS_AUTO_ERR(pt_arc_status_t::VIRTUAL_PTR_IS_NOT_ALIGNED);
            if (paddr & ReqPalign) return STATUS_AUTO_ERR(pt_arc_status_t::PHYSICAL_PTR_IS_NOT_ALIGNED);
            if (m_children[index]) return STATUS_AUTO_ERR(pt_arc_status_t::ALREADY_MAPPED);

            PT_ARC_MAP(index, paddr | translateFlags(flags, true));
            return STATUS_AUTO_OK(pt_arc_status_t::OK);
        } else {
            if (!CanChildrenHaveArcs) return STATUS_AUTO_WTF(pt_arc_status_t::PAGE_SIZE_INVALID);
            if (m_childArcs[index]) {
                return m_childArcs[index]->map(vaddr, paddr, psz, flags);
            } else {
                if (m_children[index]) return STATUS_AUTO_ERR(pt_arc_status_t::CANNOT_MAP_IN_FLAT_ZONE);
                
                auto arc = new (m_allocator.allocate()) TChildArc;
                m_childArcs[index] = arc;

                if (reinterpret_cast<TVirtPtr>(arc->m_children) & TChildArc::REQ_PALIGN) return STATUS_AUTO_WTF(pt_arc_status_t::ALLOCATED_ARC_IS_NOT_ALIGNED);

                PT_ARC_MAP(index, reinterpret_cast<TVirtPtr>(arc->m_children) | translateFlags(flags, false));

                return arc->map(vaddr, paddr, psz, flags); /* bug: redundant arc allcation when map failed */
            }
        }
    }
    status<pt_arc_status_t> unmap(TVirtPtr vaddr) {
        // should unmap() check alignment?

        auto index = (vaddr & IndexMask) >> IndexShift;
        if (m_children[index]) {
            auto arc = m_childArcs[index];
            if (arc) {
                auto status = arc->unmap(vaddr);
                if (status.isError()) return status;
                if (arc->m_mappedCount == 0) {
                    m_childArcs[index] = nullptr;

                    PT_ARC_UNMAP(index);
                    
                    arc->~TChildArc();
                    m_allocator.deallocate(arc);
                }
                return STATUS_AUTO_OK(pt_arc_status_t::OK);
            }
            else {
                PT_ARC_UNMAP(index);
                return STATUS_AUTO_OK(pt_arc_status_t::OK);
            }
        } else return STATUS_AUTO_OK(pt_arc_status_t::ALREADY_UNMAPPED);
    }
    option<TPhysPtr> getMapping(TVirtPtr vaddr, bool adjustPtr) {
        auto index = (vaddr & IndexMask) >> IndexShift;
        if (m_children[index]) {
            auto arc = m_childArcs[index];
            if (arc) return arc->getMapping(vaddr, adjustPtr);
            else {
                auto aligned = vaddr & ReqValign;
                auto phys = m_children[index] & ReqPalign;

                if (adjustPtr)
                    return option<TPhysPtr>::some(phys + (vaddr - aligned));
                else 
                    return option<TPhysPtr>::some(phys);
            }
        } else return option<TPhysPtr>::none();
    }
    status<pt_arc_status_t> apply() {
        if (!IsRoot) return STATUS_AUTO_WTF(pt_arc_status_t::CANNOT_APPLY_NON_ROOT_ARC);
#if __LP64__
        __asm__ __volatile__ (
            "lea %0, %%rax\n"
            "mov %%rax, %%cr3\n" :: "m"(m_children)
        );
#else
        __asm__ __volatile__ (
            "lea %0, %%eax\n"
            "mov %%eax, %%cr3\n" :: "m"(m_children)
        );
#endif
        return STATUS_AUTO_OK(pt_arc_status_t::OK);
    }
};

const uint64_t MAX_LONGMODE_ADDR_SPACE = 0x7FFFFFFFFFFF;

template <typename TVirtPtr, bool NoFlat = false, bool NoRsvd = false>
struct arc_direct_flag_translator_t {
    TVirtPtr translateFlags(pt_page_flags_t flags, bool flat) {
        return 
            PTSE_FLAG_PRESENT |
            (!NoRsvd && (flags & PAGE_FLAG_RW) ? PTSE_FLAG_RW : PTSE_FLAG_NONE) |
            (!NoRsvd && (flags & PAGE_FLAG_RING3) ? PTSE_FLAG_RING3 : PTSE_FLAG_NONE) |
            (!NoFlat && flat ? PTSE_FLAG_SIZE : PTSE_FLAG_NONE);
    }
};

template <typename TAllocator>
using pt64_arc_t = pt_arc_t<TAllocator, false, pt_page_size_t::SIZE_4K, 
    uint64_t, uint64_t, false, PTSE_ENTRIES64, arc_last_t<uint64_t, uint64_t>, true, 
    0xfffull & MAX_LONGMODE_ADDR_SPACE, 0xfffull & MAX_LONGMODE_ADDR_SPACE,
    12, 0x1FF000, arc_direct_flag_translator_t<uint64_t, true>>;

template <typename TAllocator>
using pd64_arc_t = pt_arc_t<TAllocator, false, pt_page_size_t::SIZE_2M, 
    uint64_t, uint64_t, true, PTSE_ENTRIES64, pt64_arc_t<TAllocator>, true, 
    0x1fffffull & MAX_LONGMODE_ADDR_SPACE, 0xfffull & MAX_LONGMODE_ADDR_SPACE,
    21, 0x3fe00000, arc_direct_flag_translator_t<uint64_t>>;

template <typename TAllocator>
using pdpt_pae_arc_t = pt_arc_t<TAllocator, true, pt_page_size_t::SIZE_LARGE, 
    uint64_t, uint64_t, true, 4, pd64_arc_t<TAllocator>, false, 
    0x3fffffffull & MAX_LONGMODE_ADDR_SPACE, 0x1full & MAX_LONGMODE_ADDR_SPACE,
    30, 0xc0000000, arc_direct_flag_translator_t<uint64_t, true, true>>;

template <typename TAllocator>
using pdpt64_arc_t = pt_arc_t<TAllocator, true, pt_page_size_t::SIZE_1G, 
    uint64_t, uint64_t, true, PTSE_ENTRIES64, pd64_arc_t<TAllocator>, true, 
    0x3fffffffull & MAX_LONGMODE_ADDR_SPACE, 0xfffull & MAX_LONGMODE_ADDR_SPACE,
    30, 0x7fc0000000, arc_direct_flag_translator_t<uint64_t>>;

template <typename TAllocator>
using pml4_arc_t = pt_arc_t<TAllocator, true, pt_page_size_t::SIZE_LARGE, 
    uint64_t, uint64_t, true, PTSE_ENTRIES64, pdpt64_arc_t<TAllocator>, true, 
    0x7fffffffffull & MAX_LONGMODE_ADDR_SPACE, 0xfffull & MAX_LONGMODE_ADDR_SPACE,
    39, 0xff8000000000, arc_direct_flag_translator_t<uint64_t>>;