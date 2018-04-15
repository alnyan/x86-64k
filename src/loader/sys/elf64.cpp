#include "elf.hpp"
#include <algo/string.hpp>

static constexpr str<5> m_elfMagic(ELFMAG);

bool elf::Elf64::isValid() const {
    return m_elfMagic == string_slice(m_ehdr.e_ident, 4);
}

int elf::Elf64::target() const {
    return m_ehdr.e_ident[EI_CLASS];
}
