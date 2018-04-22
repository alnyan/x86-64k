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
#include <dev/vesa.hpp>
#include <sys/isr.hpp>
#include <sys/gdt.hpp>

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
    debug::init();
    devices::rs232::SerialPort com1(0x3F8);
    devices::term80::TextTerminal b8;
    debug::regOutDev(&com1);
    debug::regOutDev(&b8);

    // setting up new, valid GDT (and also TSS)
    gdt::init();

    debug::printf("Entered kernel\n");
    validateLoaderData(loaderData);
    pm::retainLoaderPaging(loaderData);
    mm::init();
    pm::kernel()->dump();
    heap::init();

    // allocating 64K task-switch stack, 64-bit aligned
    const unsigned taskswitchStackSize = 0x10000;
    auto taskswitchStack = 
        reinterpret_cast<uint8_t*>(heap::kernelHeap.allocAligned(taskswitchStackSize, sizeof(uintptr_t)).orPanic("Couldn't allocate task-switch stack"));
    gdt::setTaskswitchStack(taskswitchStack + taskswitchStackSize /* passing upper bound */);

    vesa::init();

    isr_setup_handlers();
    debug::printf("firing c8!\n");
    __asm__ __volatile__ ("int $0xc8");
    debug::printf("returned from c8!\n\n");

    debug::printf("testing tss!\n");
    debug::printf("downgrading to ring3\n");

    asm volatile ( 
        "mov $0x23, %ax\n" 
        "mov %ax, %ds\n" 
        "mov %ax, %es\n" 
        "mov %rsp, %rax\n" 
        "pushq $0x23\n" 
        "pushq %rax\n"
        "pushfq\n"
        "pushq $0x1b\n"
        "pushq $1f\n"
        "iretq\n"
        "1: \n" );

    debug::printf("in usermode, calling c9\n");
    __asm__ __volatile__ ("int $0xc9");
    debug::printf("returned from c9 to usermode\n");

    while (true) {
    }
}
