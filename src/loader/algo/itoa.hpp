#pragma once
#include <stddef.h>

namespace itoa {

    using ValueType = uint64_t;
    using SignedValueType = int64_t;

    constexpr size_t bufferSize = sizeof(ValueType) * 8 + 1;
    constexpr char chars[] = "0123456789abcdef";

    template<size_t base, bool u = false> char *itoa(char *buf, ValueType val) {
        static_assert(base != 0 && base <= 16);
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
