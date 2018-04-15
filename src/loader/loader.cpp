#include <stdint.h>
#include <sys/std.hpp>
#include <mem/pm.hpp>
#include <mem/pmpae.hpp>
#include <sys/debug.hpp>
#include <sys/types.h>
#include <boot/multiboot.h>
#include <sys/elf.hpp>
#include <algo/memory.hpp>

extern "C" struct multiboot_info *mb_info_ptr;

static constexpr uintptr_t m_loadAddr = 0x400000; // Real address at which kernel will be loaded
pm::pae::Pdpt *pdp;

void load_elf(uintptr_t mod_start, size_t mod_size) {
    elf::Elf64 *elf = reinterpret_cast<elf::Elf64 *>(mod_start);
    assert(elf->isValid());
    assert(elf->target() == ELFCLASS64);

    size_t ns = elf->sectionCount();
    debug::printf("Elf: %a\n", elf);
    debug::printf("Section count: %d\n", ns);

    const Elf64_Shdr *shdrs = elf->sectionHeaders();
    uint64_t lowest = 0xFFFFFFFFFFFFFFFF;

    for (size_t i = 0; i < ns; ++i) {
        const Elf64_Shdr *section = elf->section(i);

        if (const char *name = elf->string(section->sh_name); name) {
            debug::printf("  Section: \"%s\"\n", name);
        }

        if (section->sh_flags & SHF_ALLOC) {
            if (section->sh_addr < lowest) {
                lowest = section->sh_addr;
            }
            debug::printf("    (Will be loaded at virt = %A)\n", section->sh_addr);
        }
    }

    assert(!(lowest & 0x7FFFF));

    debug::printf("Lowest virtual address: %A will be mapped to %a\n", lowest, m_loadAddr);

    // TODO: map more pages
    pdp->map(0x00400000, 0x00400000, 1 << 7);
    
    for (size_t i = 0; i < ns; ++i) {
        const Elf64_Shdr *section = elf->section(i);

        if (section->sh_flags & SHF_ALLOC) {
            uint64_t off64 = section->sh_addr - lowest;
            uint32_t off32 = static_cast<uint32_t>(off64);
            uint64_t shoff64 = section->sh_offset;
            uint32_t shoff32 = static_cast<uint32_t>(shoff64);
            uint64_t size64 = section->sh_size;
            uint32_t size32 = static_cast<uint32_t>(size64);

            debug::printf("  Loading section %A, offset %a (%a in file), to addr %a, size %a\n", section->sh_addr, off32, shoff32, m_loadAddr + off32, size32);

            const void *fileAddr = reinterpret_cast<const void *>(shoff32 + reinterpret_cast<uintptr_t>(&elf));

            memcpy(reinterpret_cast<void *>(m_loadAddr + off32), fileAddr, size32);
        }
    }

    pdp->unmap(0x00400000);
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
