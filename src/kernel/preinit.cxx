#include "../loader/loader.hpp"
#include <sys/types.h>
#include <sys/debug.hpp>
#include <algo/string.hpp>
#include <sys/paging/ptse_arc_pae.hpp>
#include <mem/mm.hpp>
#include <mem/heap.hpp>
#include <algo/new.hpp>
#include <dev/rs232.hpp>
#include <dev/term80.hpp>
#include <dev/vesa.hpp>
#include <sys/isr.hpp>
#include <sys/gdt.hpp>
#include <vector>

extern "C" void _init();

extern pml4_arc_t *arc;

class test_allocator : public arc_allocator_t {
private:
    uintptr_t m_offs = 0x304000;
public:
    void *allocate() override { 
        debug::printf("allocating\n");
        auto ptr = reinterpret_cast<void*>(m_offs);
        m_offs += 0x4000;
        return ptr;
    }
    void deallocate(void *ptr) override {}
};

extern "C" void kernel_preinit() {
    debug::init();
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
    debug::printf("allocating\n");
    arc = new (reinterpret_cast<void*>(0x300000)) pml4_arc_t(&allocator);
    uint64_t vma64 = 0x4000000000;

    debug::printf("mapping\n");
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

    while (1);
}
