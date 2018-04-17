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
        ~Heap();

        option<void *> alloc(size_t size);
        void *allocOrPanic(size_t size);
        void free(void *ptr);

        HeapHeader *rootHeader();

        void dump();

    private:
        uintptr_t m_base;
        size_t m_size;
    };

}
