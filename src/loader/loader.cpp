#include <stdint.h>
#include <sys/std.hpp>
#include <mem/pm.hpp>
#include <mem/pmpae.hpp>
#include <sys/debug.hpp>
#include <sys/types.h>
#include <boot/multiboot.h>
#include <sys/elf.hpp>
#include <algo/memory.hpp>
#include <mem/pm64.hpp>
#include "loader.hpp"

extern "C" struct multiboot_info *mb_info_ptr;
extern "C" [[noreturn]] void long_enter(uint64_t);

LoaderData loader_data;

static constexpr uintptr_t m_loadAddr = 0x400000; // Real address at which kernel will be loaded
pm::pae::Pdpt *pdp;

void load_elf(uintptr_t mod_start, size_t mod_size) {
    elf::Elf64 *elf = reinterpret_cast<elf::Elf64 *>(mod_start);
    assert(elf->isValid());
    assert(elf->target() == ELFCLASS64);

    size_t ns = elf->programCount();
    debug::printf("Elf: %a\n", elf);
    debug::printf("Section count: %d\n", ns);

    const Elf64_Phdr *phdrs = elf->programHeaders();

    // TODO: make sure entry is 2MB-page-aligned
    uint64_t entry64 = elf->entry();

    // TODO: map more pages
    pdp->map(0x00400000, 0x00400000, 1 << 7);
    pdp->apply();

    for (size_t i = 0; i < ns; ++i) {
        const Elf64_Phdr *programHeader = elf->programHeader(i);

        if (programHeader->p_type == PT_LOAD) {
            uint64_t paddr64 = programHeader->p_paddr;
            uint64_t vaddr64 = programHeader->p_vaddr;
            uint32_t paddr32 = static_cast<uint32_t>(paddr64);
            uint32_t vaddr32 = static_cast<uint32_t>(vaddr64);
            uint64_t size64 = programHeader->p_filesz;
            uint32_t size32 = static_cast<uint32_t>(size64);

            uint64_t off64 = programHeader->p_offset;
            uint32_t off32 = static_cast<uint32_t>(off64);

            const void *l = reinterpret_cast<const void *>(mod_start + off32);

            debug::printf("paddr32 = %a\n", paddr32);

            memcpy(reinterpret_cast<void *>(paddr32), l, size32);//FIXME:!
        }
    }
    pdp->unmap(0x00400000);

    // Convert pdp to pml4
    pm_disable();

    pm::pm64::Pml4 *pml = new (0x100000) pm::pm64::Pml4;
    pml->map(0x0, 0x0, 1 << 7);
    pml->map(0x200000, 0x200000, 1 << 7);
    pml->map(entry64, 0x400000, 1 << 7); // TODO: map more pages if needed
    pml->apply();

    debug::printf("Entry: %A\n", entry64);
    long_enter(entry64);
}

extern "C" void loader_main(void) {
    // Map loader here
    pm::setAlloc(0x100000); // Because pdpt
    pdp = new (0x100000) pm::pae::Pdpt;
    pdp->map(0x00000000, 0x00000000, 1 << 7);
    pdp->map(0x00200000, 0x00200000, 1 << 7);

    pdp->apply();
    pae_enable();
    pm_enable();

    // Find kernel
    struct multiboot_mod_list *mb_mod = reinterpret_cast<struct multiboot_mod_list *>(mb_info_ptr->mods_addr);
    size_t mb_mod_count = mb_info_ptr->mods_count;
    assert(mb_mod_count == 1);
    load_elf(mb_mod->mod_start, mb_mod->mod_end - mb_mod->mod_start);

    while (1) {
        __asm__ __volatile__ ("cli; hlt");
    }
}
