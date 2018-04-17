#include "heap.hpp"
#include <algo/memory.hpp>

heap::Heap::Heap(uintptr_t base, size_t size): m_base{base}, m_size{size} {
    // Place initial header
    HeapHeader *hdr = rootHeader();
    hdr->flags = headerMagic;
    hdr->length = size - sizeof(HeapHeader);
    hdr->prev = 0;
    hdr->next = 0;
}

heap::Heap::~Heap() {
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

void heap::Heap::dump() {
    debug::printf("heap::dump(%la)\n", m_base);
    for (HeapHeader *hdr = rootHeader(); hdr; hdr = hdr->next) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(hdr) + sizeof(HeapHeader);

        debug::printf(" * %la - %la, %s\n", addr, addr + hdr->length, (hdr->flags & heap::HeapFlags::HF_ALLOC) ? "USED" : "FREE");
    }
}

void *heap::Heap::allocOrPanic(size_t s) {
    return alloc(s).orPanic("Failed to allocate object on heap");
}
