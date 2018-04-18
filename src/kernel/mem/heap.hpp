#pragma once
#include <sys/types.h>
#include <stdint.h>
#include <algo/option.hpp>

namespace heap {

    enum HeapFlags: uint64_t {
        HF_ALLOC    = 1
    };

    static constexpr HeapFlags headerMagic = HeapFlags(0x1A110C00);

    struct HeapHeader {
        uint64_t flags;
        size_t length;
        HeapHeader *next, *prev;
    };

    class Heap {
    public:
        Heap(uintptr_t base, size_t size);
        Heap();
        ~Heap();

        option<void *> alloc(size_t size);
        option<void *> allocAligned(size_t size, size_t align);
        void *allocOrPanic(size_t size);
        void free(void *ptr);
        void freeChecked(void *ptr, size_t sz);

        HeapHeader *rootHeader();

        void dump();
        void reset(uintptr_t base, size_t size);

        bool valid() const;

    private:
        uintptr_t m_base;
        size_t m_size;
    };

    extern Heap kernelHeap;

    void init();

}
