#pragma once

#include <sys/debug.hpp>
#include <cstdint>

class test_allocator {
private:
    static uintptr_t m_offs;
public:
    void *allocate() { 
        debug::dprintf("allocating\n");
        auto ptr = reinterpret_cast<void*>(m_offs);
        m_offs += 0x4000;
        return ptr;
    }
    void deallocate(void *ptr) {}
};

using ptse_allocator_t = test_allocator;