#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <algo/string.hpp>
#include <sys/paging/ptse_arc_base.hpp>
#include <mem/mm.hpp>
#include <mem/ptse_allocator.hpp>
#include <mem/heap.hpp>
#include <algo/new.hpp>
#include <dev/rs232.hpp>
#include <dev/term80.hpp>
#include <dev/vesa.hpp>
#include <sys/isr.hpp>
#include <sys/gdt.hpp>
#include <iostream>
#include <vector>

extern "C" void __cxa_atexit() {
    // C++ trying to register destructor
    // Just ignore this
}

extern "C" void __cxa_pure_virtual() {
    debug::dpanic("__cxa_pure_virtual: pure virtual call encountered.");
}

extern "C" void kernel_main() {
    /*debug::init();
    devices::rs232::SerialPort com1(0x3F8);
    devices::term80::TextTerminal b8;
    debug::regOutDev(&com1);
    debug::regOutDev(&b8);

    // setting up new, valid GDT (and also TSS)
    gdt::init();

    debug::printf("Entered kernel\n");
    
    // not necessary at this point
    // validateLoaderData(loaderData);

    // temporary paging 

    debug::printf("setting up temporary pml4 at wrong place\n");
    test_allocator allocator;
    arc = new (reinterpret_cast<void*>(0x300000)) pml4_arc_t(&allocator);
    uint64_t vma64 = 0x4000000000;

    arc->map(0x0, 0x0, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3));
    arc->map(0x200000, 0x200000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3));
    arc->map(vma64,            0x1000000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x200000, 0x1200000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x400000, 0x1400000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x600000, 0x1600000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x800000, 0x1800000, pml4_arc_t::LEVEL_2M, page_struct_flags_t(PTSE_FLAG_RW | PTSE_FLAG_RING3)); // TODO: map more pages if needed
    arc->apply();
    debug::printf("done\n");

    mm::init();
    heap::init(arc);

    // allocating 64K task-switch stack, 64-bit aligned
    const unsigned taskswitchStackSize = 0x10000;
    auto taskswitchStack = 
        reinterpret_cast<uint8_t*>(heap::kernelHeap.allocAligned(taskswitchStackSize, sizeof(uintptr_t)).orPanic("Couldn't allocate task-switch stack"));
    gdt::setTaskswitchStack(taskswitchStack + taskswitchStackSize); // passing upper bound

    // vesa::init();

    debug::printf("enabling fpu+sse\n");
    asm volatile (
        "push %rax\n"
        "mov %cr0, %eax\n"
        "and $0xfffffffd, %eax\n" // turning off CR0.EM 
        "or $0x00000002, %eax\n" // turning on CR0.MP
        "mov %eax, %cr0\n"
        "mov %cr4, %eax\n"
        "or $0x00000600, %eax\n" // enabling CR4.OSFXSR and CR4.OSXMMEXCPT
        "mov %eax, %cr4\n"
        "pop %rax"
    );

    debug::printf("testing stdc++\n");
    std::cout << "shit printing with <iostream>\n";
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "vector of values incoming: ";
    for (auto v: vec) std::cout << v << " ";
    std::cout << std::endl;

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
        "movabsq $1f, %rax\n"
        "pushq %rax\n"
        "iretq\n"
        "1: \n" );

    debug::printf("in usermode, calling c9\n");
    __asm__ __volatile__ ("int $0xc9");
    debug::printf("returned from c9 to usermode\n");

    while (true) {
    }*/
}
