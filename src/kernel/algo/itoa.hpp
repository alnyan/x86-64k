#pragma once
#include <sys/types.h>

namespace itoa {

    /// Maximum unsigned value type for this platform (64 bits)
    using ValueType = uint64_t;
    /// Maximum signed value type for this platform (64 bits)
    using SignedValueType = int64_t;

    /// Maximum number of characters needed to store the result
    constexpr size_t bufferSize = sizeof(ValueType) * 8 + 1;

    /// Array of digit characters
    constexpr char chars[] = "0123456789abcdef";

    /**
     * \brief Converts numerical integer value to string representation
     * \param buf           - Output string buffer, must be able to store all digits plus NULL-terminator
     * \param val           - Value to convert
     * \param base          - Base (2/10/16/etc.), max = 16, min = 2
     * \param u             - If base = 10, forces unsigned mode
     * \return Pointer to the beginning of the buffer
     */
    template<size_t base, bool u = false> char *itoa(char *buf, ValueType val) {
        static_assert(base >= 2 && base <= 16);
        if (!val) {
            buf[0] = '0';
            buf[1] = 0;
            return buf;
        }

        bool sign;
        size_t n = 0, r, j;

        if constexpr (base == 10 && !u) {
            if (static_cast<SignedValueType>(val) < 0) {
                sign = true;
                val = static_cast<ValueType>(-static_cast<SignedValueType>(val));
            } else {
                sign = false;
            }
        }

        while (val) {
            buf[n++] = chars[val % base];
            val /= base;
        }

        if constexpr (base == 10 && !u) {
            if (sign) {
                buf[n] = '-';
                r = n + 1;
            } else {
                r = n--;
            }
        } else {
            r = n--;
        }

        j = 0;
        while (n > j) {
            buf[n] ^= buf[j];
            buf[j] ^= buf[n];
            buf[n] ^= buf[j];
            --n;
            ++j;
        }

        buf[r] = 0;

        return buf;
    }

}
