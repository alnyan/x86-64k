#include <stdint.h>
#include <sys/std.hpp>
#include <mem/pm.hpp>
#include <mem/pmpae.hpp>
#include <sys/debug.hpp>
#include <sys/types.h>
#include <boot/multiboot.h>
#include <sys/elf.hpp>

extern "C" struct multiboot_info *mb_info_ptr;

void load_elf(uintptr_t mod_start, size_t mod_size) {
    elf::Elf64 *elf = reinterpret_cast<elf::Elf64 *>(mod_start);
    assert(elf->isValid());
    assert(elf->target() == ELFCLASS64);
}

extern "C" void loader_main(void) {
    // Map loader here
    pm::setAlloc(0x100000); // Because pdpt
    pm::pae::Pdpt *pdp = new (0x100000) pm::pae::Pdpt;
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
