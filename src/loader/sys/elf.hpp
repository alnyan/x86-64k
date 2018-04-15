#pragma once
#include <sys/elf.h>

namespace elf {

    class Elf64 {
    public:
        bool isValid() const;
        int target() const;

    private:

        Elf64_Ehdr m_ehdr;
    };

}
