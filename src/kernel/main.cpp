#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <algo/string.hpp>
#include <mem/pm.hpp>
#include <mem/mm.hpp>
#include <mem/heap.hpp>

void validateLoaderData(LoaderData *data) {
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(LoaderData); ++i) {
        sum += reinterpret_cast<const uint8_t *>(data)[i];
    }

    assert((sum & 0xFF) == 0);
}

extern "C" void kernel_main(LoaderData *loaderData) {
    debug::printf("Entered kernel\n");
    validateLoaderData(loaderData);
    pm::retainLoaderPaging(loaderData);
    mm::init();

    pm::kernel()->dump();

    auto heapRegion = mm::alloc(pm::kernel(), 16, mm::AllocFlagsType::AF_RW).orPanic("Failed to allocate memory for heap");
    
    {
        heap::Heap h(heapRegion, 16 * 0x200000);

        auto p0 = h.alloc(17 * 0x200000);

        if (!p0) {
            debug::printf("We failed, as expected\n");
        }

        size_t chunkSizes[] = {
            12, 23, 34, 45, 56, 67, 78
        };
        void *ptrs[sizeof(chunkSizes) / sizeof(size_t)];

        for (size_t i = 0; i < sizeof(chunkSizes) / sizeof(size_t); ++i) {
            ptrs[i] = h.allocOrPanic(chunkSizes[i]);
        }
        h.dump();

        for (size_t i = 0; i < sizeof(chunkSizes) / sizeof(size_t); ++i) {
            h.free(ptrs[i]);
        }
        
        h.dump();
    }

    mm::free(pm::kernel(), heapRegion, 16);

    while (true) {
    }
}
