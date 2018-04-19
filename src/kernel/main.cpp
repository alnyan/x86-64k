#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <algo/string.hpp>
#include <mem/pm.hpp>
#include <mem/mm.hpp>
#include <mem/heap.hpp>
#include <algo/new.hpp>
#include <algo/memory.hpp>

void validateLoaderData(LoaderData *data) {
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(LoaderData); ++i) {
        sum += reinterpret_cast<const uint8_t *>(data)[i];
    }

    assert((sum & 0xFF) == 0);
}

extern "C" void __cxa_atexit() {
    // Won't happen
}

extern "C" void kernel_main(LoaderData *loaderData) {
    debug::printf("Entered kernel\n");
    validateLoaderData(loaderData);
    pm::retainLoaderPaging(loaderData);
    mm::init();
    pm::kernel()->dump();
    heap::init();

    while (true) {
    }
}
