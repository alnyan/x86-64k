#pragma once

#include <stdint.h>

namespace emu {

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

        // segmented memory management
        uint8_t  peekb(uint16_t segment, uint16_t offset);
        uint16_t peekw(uint16_t segment, uint16_t offset);
        uint32_t peekd(uint16_t segment, uint16_t offset);
        
        void pokeb(uint16_t segment, uint16_t offset, uint8_t  value);
        void pokew(uint16_t segment, uint16_t offset, uint16_t value);
        void poked(uint16_t segment, uint16_t offset, uint32_t value);
    };

}