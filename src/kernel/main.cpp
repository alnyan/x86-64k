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

        void *p0 = h.allocOrPanic(12);
        void *p1 = h.allocOrPanic(23);
        void *p2 = h.allocOrPanic(34);
        h.dump();

        h.free(p1);
        h.dump();

        h.free(p0);
        h.dump();

        h.free(p2);
        h.dump();
    }

    mm::free(pm::kernel(), heapRegion, 16);

    while (true) {
    }
}
