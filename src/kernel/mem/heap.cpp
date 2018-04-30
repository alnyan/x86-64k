#include "heap.hpp"
#include <cstring>
#include <algo/algorithm.hpp>
#include <mem/mm.hpp>

heap::Heap heap::kernelHeap;

heap::Heap::Heap(uintptr_t base, size_t size): m_base{base}, m_size{size} {
    // Place initial header
    HeapHeader *hdr = rootHeader();
    hdr->flags = headerMagic;
    hdr->length = size - sizeof(HeapHeader);
    hdr->prev = 0;
    hdr->next = 0;
}

heap::Heap::Heap(): m_base{0}, m_size{0} {
}

heap::Heap::~Heap() {
    if (m_size) {
        // Make sure no objects are allocated when heap dies
        for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
            if ((hdr->flags & heap::headerMagic) != heap::headerMagic) {
                panic_msg("Heap is broken");
            }
            if (hdr->flags & heap::HeapFlags::HF_ALLOC) {
                panic_msg("Tried to destroy heap with living objects");
            }
        }
    }
}

void heap::Heap::reset(uintptr_t base, size_t size) {
    if (m_size) {
        for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
            if ((hdr->flags & heap::headerMagic) != heap::headerMagic) {
                panic_msg("Heap is broken");
            }
            if (hdr->flags & heap::HeapFlags::HF_ALLOC) {
                panic_msg("Tried to destroy heap with living objects");
            }
        }
    }

    m_base = base;
    m_size = size;

    // Place initial header
    HeapHeader *hdr = rootHeader();
    hdr->flags = headerMagic;
    hdr->length = size - sizeof(HeapHeader);
    hdr->prev = 0;
    hdr->next = 0;  
}

heap::HeapHeader *heap::Heap::rootHeader() {
    return reinterpret_cast<heap::HeapHeader *>(m_base);
}

option<void *> heap::Heap::alloc(size_t size) {
    debug::printf("heap::alloc(%la) %lu\n", m_base, size);

    for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
        if ((hdr->flags & heap::headerMagic) != heap::headerMagic) {
            panic_msg("Heap is broken");
        }
        if (hdr->flags & heap::HeapFlags::HF_ALLOC) {
            continue;
        }

        uintptr_t addr = reinterpret_cast<uintptr_t>(hdr) + sizeof(HeapHeader);

        if (hdr->length > size + sizeof(HeapHeader)) {
            // Split into two parts
            hdr->flags |= heap::HeapFlags::HF_ALLOC;

            HeapHeader *newHeader = reinterpret_cast<HeapHeader *>(addr + size);
            newHeader->prev = hdr;
            newHeader->next = hdr->next;
            newHeader->flags = heap::headerMagic;
            newHeader->length = hdr->length - size - sizeof(HeapHeader);

            hdr->next = newHeader;
            hdr->length = size;

            return option<void *>::some(reinterpret_cast<void *>(addr));
        } else if (hdr->length == size) {
            // Allocate whole chunk
            hdr->flags |= heap::HeapFlags::HF_ALLOC;

            return option<void *>::some(reinterpret_cast<void *>(addr));
        }
    }

    return option<void *>::none();
}

option<void *> heap::Heap::allocAligned(size_t size, size_t align) {
    debug::printf("heap::allocAligned(%la) %lu, align=0x%lx\n", m_base, size, align);
    // TODO: check that align is power of two

    for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
        if ((hdr->flags & heap::headerMagic) != heap::headerMagic) {
            panic_msg("Heap is broken");
        }
        if (hdr->flags & heap::HeapFlags::HF_ALLOC) {
            continue;
        }

        uintptr_t addr = reinterpret_cast<uintptr_t>(hdr) + sizeof(HeapHeader);
        uintptr_t nextAligned = alignUp(addr, align);

        while (1) {
            size_t padSize = nextAligned - addr;
            ssize_t actualSizes = hdr->length - padSize;
            if (actualSizes < 0) {
                break;
            }
            size_t actualSize = static_cast<size_t>(actualSizes);

            if (addr == nextAligned) {
                // The block is already aligned

                if (actualSize == size) {
                    // No splits required
                    hdr->flags |= heap::HeapFlags::HF_ALLOC;

                    return option<void *>::some(reinterpret_cast<void *>(addr));
                } else {
                    // Cut end, return beginning
                    hdr->flags |= heap::HeapFlags::HF_ALLOC;

                    HeapHeader *newHeader = reinterpret_cast<HeapHeader *>(addr + size);
                    newHeader->prev = hdr;
                    newHeader->next = hdr->next;
                    newHeader->flags = heap::headerMagic;
                    newHeader->length = hdr->length - size - sizeof(HeapHeader);

                    hdr->next = newHeader;
                    hdr->length = size;

                    return option<void *>::some(reinterpret_cast<void *>(addr));
                }
            } else {
                // Make sure we can fit a header
                if (padSize < sizeof(HeapHeader)) {
                    nextAligned += align;

                    if (nextAligned - addr >= hdr->length) {
                        break;
                    }
                    continue;
                }

                // Cut padding
                if (actualSize > size + sizeof(HeapHeader)) {
                    // Cut end, return middle
                    HeapHeader *end = reinterpret_cast<HeapHeader *>(nextAligned + size);
                    HeapHeader *mid = reinterpret_cast<HeapHeader *>(nextAligned - sizeof(HeapHeader));

                    end->next = hdr->next;
                    mid->next = end;
                    hdr->next = mid;
                    end->length = actualSize - size - sizeof(HeapHeader);
                    mid->length = size;
                    hdr->length = padSize - sizeof(HeapHeader);
                    end->flags = heap::headerMagic;
                    mid->flags = heap::headerMagic | heap::HeapFlags::HF_ALLOC;
                    mid->prev = hdr;
                    end->prev = mid;

                    return option<void *>::some(reinterpret_cast<void *>(nextAligned));
                } else if (actualSize == size) {
                    // Cut nothing more, return end
                    HeapHeader *end = reinterpret_cast<HeapHeader *>(nextAligned - sizeof(HeapHeader));

                    end->prev = hdr;
                    end->next = hdr->next;
                    hdr->next = end;

                    end->length = size;
                    hdr->length = padSize - sizeof(HeapHeader);

                    end->flags = heap::headerMagic | heap::HeapFlags::HF_ALLOC;

                    return option<void *>::some(reinterpret_cast<void *>(nextAligned));    
                }
            }
        }
    }

    return option<void *>::none();
}

void heap::Heap::free(void *p) {
    debug::printf("heap::free(%la) %la\n", m_base, p);
    uintptr_t addr = reinterpret_cast<uintptr_t>(p);
    HeapHeader *hdr = reinterpret_cast<HeapHeader *>(addr - sizeof(HeapHeader));

    assert((hdr->flags & heap::headerMagic) == heap::headerMagic);

    if (!(hdr->flags & heap::HeapFlags::HF_ALLOC)) {
        panic_msg("Heap object double-free detected: tried to free non-allocated pointer");
    }

    hdr->flags &= ~heap::HeapFlags::HF_ALLOC;

    while (hdr->prev) {
        HeapHeader *prev = hdr->prev;
        assert((prev->flags & headerMagic) == headerMagic);

        if (prev->flags & heap::HeapFlags::HF_ALLOC) {
            break;
        }

        hdr = prev;
    }

    while (hdr->next) {
        HeapHeader *next = hdr->next;
        assert((next->flags & headerMagic) == headerMagic);

        if (next->flags & heap::HeapFlags::HF_ALLOC) {
            break;
        }

        hdr->length += sizeof(HeapHeader) + next->length;
        hdr->next = next->next;
        if (hdr->next) {
            hdr->next->prev = hdr;
        }
        memset(next, 0, sizeof(HeapHeader));
    }
}

void heap::Heap::freeChecked(void *ptr, size_t sz) {
    debug::printf("heap::freeChecked(%la) %la, sz = %lu\n", m_base, ptr, sz);
    uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);
    HeapHeader *hdr = reinterpret_cast<HeapHeader *>(addr - sizeof(HeapHeader));
    assert((hdr->flags & heap::headerMagic) == heap::headerMagic);

    assert(sz == hdr->length);

    free(ptr);
}

void heap::Heap::dump() {
    debug::printf("heap::dump(%la)\n", m_base);
    for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(hdr) + sizeof(HeapHeader);

        debug::printf(" * %la - %la, %s\n", addr, addr + hdr->length, (hdr->flags & heap::HeapFlags::HF_ALLOC) ? "USED" : "FREE");
    }
}

bool heap::Heap::valid() const {
    return m_size && m_base;
}

void heap::init(pml4_arc_t *arc) {
    auto heapRegion = mm::alloc(arc, 16, mm::AllocFlagsType::AF_RW).orPanic("Failed to allocate memory for kernel heap");
    kernelHeap.reset(heapRegion, 16 * 0x200000);
    debug::printf("Created heap at %la, size %lu\n", heapRegion, 16 * 0x200000);
}
