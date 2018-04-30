#include "elf.hpp"
#include <nostdcxx/string.hpp>

bool elf::Elf64::isValid() const {
    return !strncmp(ELFMAG, reinterpret_cast<const char *>(m_ehdr.e_ident), 4);
}

int elf::Elf64::target() const {
    return m_ehdr.e_ident[EI_CLASS];
}

const char *elf::Elf64::string(int offset) const {
    const char *strtab = strTable();
    if (!strtab) {
        return nullptr;
    }
    return strtab + offset;
}

size_t elf::Elf64::sectionCount() const {
    return static_cast<size_t>(m_ehdr.e_shnum);
}

size_t elf::Elf64::programCount() const {
    return static_cast<size_t>(m_ehdr.e_phnum);
}

Elf64_Addr elf::Elf64::entry() const {
    return m_ehdr.e_entry;
}
