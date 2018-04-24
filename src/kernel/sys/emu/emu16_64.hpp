#pragma once

#include <stdint.h>
#include <sys/panic.hpp>

namespace emu {

    const int NOT_INDEXED = 8;
    const int DS_INDEX = 1;
    const int CS_INDEX = 3;

    class Emulator16 {
    public:
        Emulator16();

        // common registers
        uint32_t eax, ebx, ecx, edx, esi, edi;
        // not very common registers
        uint32_t ebp, esp, eip, eflags;
        // segment registers
        uint16_t cs, ss, ds, es, fs, gs;

        // fetching and executing single instruction
        void step();

        // register access by index
        void *regPtrByIx(int index) {
            switch (index) {
                case 0: return &eax;
                case 1: return &ecx;
                case 2: return &edx;
                case 3: return &ebx;
                case 4: return &esp;
                case 5: return &ebp;
                case 6: return &esi;
                case 7: return &edi;
            }

            panic_msg("invalid index");
        }

        uint16_t segValueByIx(int index) {
            switch (index) {
                case DS_INDEX: return ds;
                case CS_INDEX: return cs;
            }

            panic_msg("invalid index");
        }

        void pushWordOnStack(uint16_t word) {
            esp -= sizeof(uint16_t);
            pokew(ss, esp, word);
        }

        void pushDwordOnStack(uint32_t dword) {
            esp -= sizeof(uint32_t);
            poked(ss, esp, dword);
        }

        // segmented memory management
        uint8_t  peekb(uint16_t segment, uint16_t offset);
        uint16_t peekw(uint16_t segment, uint16_t offset);
        uint32_t peekd(uint16_t segment, uint16_t offset);
        
        void pokeb(uint16_t segment, uint16_t offset, uint8_t  value);
        void pokew(uint16_t segment, uint16_t offset, uint16_t value);
        void poked(uint16_t segment, uint16_t offset, uint32_t value);
    };

}