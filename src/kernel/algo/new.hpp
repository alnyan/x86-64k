#pragma once
#include <stdint.h>
#include <sys/types.h>
#include <mem/heap.hpp>

/*inline void *operator new(size_t v) throw() {
    assert(heap::kernelHeap.valid());
    return heap::kernelHeap.alloc(v).orPanic("Allocation failed");
}

inline void *operator new[](size_t v) throw() {
    assert(heap::kernelHeap.valid());
    return heap::kernelHeap.alloc(v).orPanic("Allocation failed");
}

inline void operator delete(void *v) throw() {
    assert(heap::kernelHeap.valid());
    heap::kernelHeap.free(v);
}

inline void operator delete(void *v, size_t n) throw() {
    assert(heap::kernelHeap.valid());
    heap::kernelHeap.freeChecked(v, n);
}

inline void operator delete [](void *v) throw() {
    assert(heap::kernelHeap.valid());
    heap::kernelHeap.free(v);
}

inline void operator delete [](void *v, size_t n) throw() {
    assert(heap::kernelHeap.valid());
    heap::kernelHeap.freeChecked(v, n);
}*/