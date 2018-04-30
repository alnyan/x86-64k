#pragma once
#include <sys/elf.h>
#include <nostdcxx/string.hpp>

namespace elf {

    class Elf64 {
    public:
        bool isValid() const;
        int target() const;

        size_t sectionCount() const;
        size_t programCount() const;

        Elf64_Addr entry() const;
        
        const char *string(int offset) const;
        
        const Elf64_Shdr *section(int idx) const {
            return &sectionHeaders()[idx];
        }

        const Elf64_Phdr *programHeader(int idx) const {
            return &programHeaders()[idx];
        }

        const Elf64_Shdr *sectionHeaders() const {
            return reinterpret_cast<Elf64_Shdr *>(reinterpret_cast<uintptr_t>(&m_ehdr) + static_cast<uintptr_t>(m_ehdr.e_shoff));
        }

        const Elf64_Phdr *programHeaders() const {
            return reinterpret_cast<Elf64_Phdr *>(reinterpret_cast<uintptr_t>(&m_ehdr) + static_cast<uintptr_t>(m_ehdr.e_phoff));
        }

        const char *strTable() const {
            if (m_ehdr.e_shstrndx == SHN_UNDEF) return nullptr;
            return reinterpret_cast<const char *>(&m_ehdr) + static_cast<uintptr_t>(section(m_ehdr.e_shstrndx)->sh_offset);
        }

        const Elf64_Ehdr *data() const {
            return &m_ehdr;
        }

    private:

        Elf64_Ehdr m_ehdr;
    };

}
