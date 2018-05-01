#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <sys/lodebug.hpp>
#include <algo/string.hpp>
#include <sys/paging/ptse_arc_base.hpp>
#include <mem/ptse_allocator.hpp>
#include <mem/mm.hpp>
#include <mem/heap.hpp>
#include <algo/new.hpp>
#include <dev/rs232.hpp>
#include <dev/term80.hpp>
#include <dev/vesa.hpp>
#include <sys/isr.hpp>
#include <sys/gdt.hpp>
#include <vector>
#include <opt/status.hpp>

pml4_arc_t<ptse_allocator_t> *arc;

extern "C" void kernel_preinit() {
    debug::init();
    devices::rs232::SerialPort com1(0x3F8);
    devices::term80::TextTerminal b8;
    debug::regOutDev(&com1);
    debug::regOutDev(&b8);

    // setting up new, valid GDT (and also TSS)
    gdt::init();

    debug::dprintf("Entered kernel\n");
    
    // not necessary at this point
    // validateLoaderData(loaderData);

    // temporary paging 

    debug::dprintf("setting up temporary pml4 at wrong place\n");
    test_allocator allocator;
    debug::dprintf("allocating\n");
    arc = new (reinterpret_cast<void*>(0x300000)) pml4_arc_t<ptse_allocator_t>;
    uint64_t vma64 = 0x4000000000;

    debug::dprintf("mapping\n");
    arc->map(0x0, 0x0, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3));
    arc->map(0x200000, 0x200000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3));
    arc->map(vma64,            0x1000000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x200000, 0x1200000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x400000, 0x1400000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x600000, 0x1600000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3)); // TODO: map more pages if needed
    arc->map(vma64 + 0x800000, 0x1800000, pt_page_size_t::SIZE_2M, pt_page_flags_t(PAGE_FLAG_RW | PAGE_FLAG_RING3)); // TODO: map more pages if needed
    arc->apply();
    debug::dprintf("done\n");

    mm::init();
    heap::init(arc);

    // allocating 64K task-switch stack, 64-bit aligned
    const unsigned taskswitchStackSize = 0x10000;
    auto taskswitchStack = 
        reinterpret_cast<uint8_t*>(heap::kernelHeap.allocAligned(taskswitchStackSize, sizeof(uintptr_t)).orPanic("Couldn't allocate task-switch stack"));
    gdt::setTaskswitchStack(taskswitchStack + taskswitchStackSize); // passing upper bound
    
    debug::dprintf("enabling fpu+sse\n");
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

    while (1);
}
