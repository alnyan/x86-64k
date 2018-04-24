#pragma once

#include <stdint.h>

namespace emu {
    namespace mm {
        const unsigned SEGMENT_MULTIPLIER = 0x10;
        const unsigned SEGMENT_BOUNDARY = 0x10000;

        static inline uintptr_t addrToLinear(uint16_t segment, uint16_t offset) {
            return segment * SEGMENT_MULTIPLIER + offset;
        }
        static inline uintptr_t extractOffset(uint16_t segment, uintptr_t ptr) {
            return ptr - segment * SEGMENT_MULTIPLIER;
        }

        static inline uint8_t peekb(uint16_t segment, uint16_t offset) {
            return *reinterpret_cast<uint8_t*>(addrToLinear(segment, offset));
        }
        static inline uint16_t peekw(uint16_t segment, uint16_t offset) {
            return *reinterpret_cast<uint16_t*>(addrToLinear(segment, offset));
        }
        static inline uint32_t peekd(uint16_t segment, uint16_t offset) {
            return *reinterpret_cast<uint32_t*>(addrToLinear(segment, offset));
        }
        
        static inline void pokeb(uint16_t segment, uint16_t offset, uint8_t value) {
            *reinterpret_cast<uint8_t*>(addrToLinear(segment, offset)) = value;
        }
        static inline void pokew(uint16_t segment, uint16_t offset, uint16_t value) {
            *reinterpret_cast<uint16_t*>(addrToLinear(segment, offset)) = value;
        }
        static inline void poked(uint16_t segment, uint16_t offset, uint32_t value) {
            *reinterpret_cast<uint32_t*>(addrToLinear(segment, offset)) = value;
        }
    }
}