#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <algo/string.hpp>
#include <mem/pm.hpp>
#include <mem/mm.hpp>
#include <mem/heap.hpp>
#include <algo/new.hpp>
#include <dev/rs232.hpp>
#include <dev/term80.hpp>

void validateLoaderData(LoaderData *data) {
    uint32_t sum = 0;
    for (size_t i = 0; i < sizeof(LoaderData); ++i) {
        sum += reinterpret_cast<const uint8_t *>(data)[i];
    }

    assert((sum & 0xFF) == 0);
}

extern "C" void __cxa_atexit() {
    // C++ trying to register destructor
    // Just ignore this
}

extern "C" void __cxa_pure_virtual() {
    panic_msg("__cxa_pure_virtual: pure virtual call encountered.");
}

extern "C" void kernel_main(LoaderData *loaderData) {
    devices::rs232::SerialPort com1(0x3F8);
    devices::term80::TextTerminal b8;
    debug::regOutDev(&com1);
    debug::regOutDev(&b8);

    debug::printf("Entered kernel\n");
    validateLoaderData(loaderData);
    pm::retainLoaderPaging(loaderData);
    mm::init();
    pm::kernel()->dump();
    heap::init();

    while (true) {
    }
}
