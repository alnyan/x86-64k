#pragma once
#include <sys/emu/templates.hpp>
#include <sys/debug.hpp>

namespace emu {
    namespace alu {

        enum alu_flags_t : uint32_t {
            ALU_FLAG_NONE     = 0,
            ALU_FLAG_CARRY    = 1 << 0,
            ALU_FLAG_PARITY   = 1 << 2,
            ALU_FLAG_ADJUST   = 1 << 4,
            ALU_FLAG_ZERO     = 1 << 6,
            ALU_FLAG_SIGN     = 1 << 7,
            ALU_FLAG_OVERFLOW = 1 << 11
        };

        template <typename T> struct alu_result_t {
            T result;
            alu_flags_t newFlags;
        };

        #define UINT_MAX_VALUE(type) (((static_cast<type>(1) << (sizeof(type) * 8 - 1)) * 2) + 1)

        #define UINT_SIGN_BIT_MASK(type) (static_cast<type>(1) << (sizeof(type) * 8 - 1))

        #define INT_MAX_POSITIVE_VALUE(type) (UINT_SIGN_BIT_MASK(type) - 1)
        #define INT_MIN_NEGATIVE_VALUE(type) (static_cast<int_similar_t<type>>(UINT_SIGN_BIT_MASK(type)))

        #define HAS_OVERFLOW(value, type) (value > UINT_MAX_VALUE(type))

        static inline bool calculateParity(uint8_t lowerByte) {
            bool parity = true;
            for (int i = 0; i < 8; i++) parity ^= !!(lowerByte & (1 << i));

            return parity;
        }

        template <typename T> static ensured_uint_t<T, alu_result_t<T>> add(T lhs, T rhs, alu_flags_t prevFlags) {
            auto intLhs = static_cast<int_similar_t<T>>(lhs);
            auto intRhs = static_cast<int_similar_t<T>>(rhs);

            int_larger_t<decltype(intLhs)> intLargerLhs = intLhs;
            intLargerLhs += intRhs;

            uint_larger_t<T> largerLhs = intLargerLhs;

            ensured_uint_t<T, alu_result_t<T>> result;
            result.result = largerLhs;
            result.newFlags =  // TODO or not TODO: Adjust flag
                (HAS_OVERFLOW(largerLhs, T) ? ALU_FLAG_CARRY : 0) |
                (calculateParity(largerLhs) ? ALU_FLAG_PARITY : 0) |
                (result.result == 0 ? ALU_FLAG_ZERO : 0) |
                ((result.result & UINT_SIGN_BIT_MASK(T)) ? ALU_FLAG_SIGN : 0) |
                (intLargerLhs >= 0 
                    ? intLargerLhs > INT_MAX_POSITIVE_VALUE(T) ? ALU_FLAG_OVERFLOW : 0
                    : intLargerLhs < INT_MIN_NEGATIVE_VALUE(T) ? ALU_FLAG_OVERFLOW : 0);
        }

        template <typename T> static ensured_uint_t<T, alu_result_t<T>> sub(T lhs, T rhs, alu_flags_t prevFlags) {
            auto intLhs = static_cast<int_similar_t<T>>(lhs);
            auto intRhs = static_cast<int_similar_t<T>>(rhs);

            int_larger_t<decltype(intLhs)> intLargerLhs = intLhs;
            intLargerLhs -= intRhs;

            uint_larger_t<T> largerLhs = intLargerLhs;

            ensured_uint_t<T, alu_result_t<T>> result;
            result.result = largerLhs;
            result.newFlags = static_cast<alu_flags_t>( // TODO or not TODO: Adjust flag
                (HAS_OVERFLOW(largerLhs, T) ? ALU_FLAG_CARRY : ALU_FLAG_NONE) |
                (calculateParity(largerLhs) ? ALU_FLAG_PARITY : ALU_FLAG_NONE) |
                (result.result == 0 ? ALU_FLAG_ZERO : ALU_FLAG_NONE) |
                ((result.result & UINT_SIGN_BIT_MASK(T)) ? ALU_FLAG_SIGN : ALU_FLAG_NONE) |
                (intLargerLhs >= 0 
                    ? intLargerLhs > INT_MAX_POSITIVE_VALUE(T) ? ALU_FLAG_OVERFLOW : ALU_FLAG_NONE
                    : intLargerLhs < INT_MIN_NEGATIVE_VALUE(T) ? ALU_FLAG_OVERFLOW : ALU_FLAG_NONE));
            debug::printf("new flags: %x\n", result.newFlags);
            return result;
        }
    }
}