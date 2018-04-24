#pragma once

#include <stdint.h>

template <typename TMaybeUint, typename TShouldBe> struct ensured_uint_t;
template <typename TShouldBe> struct ensured_uint_t<uint8_t,  TShouldBe> : public TShouldBe {};
template <typename TShouldBe> struct ensured_uint_t<uint16_t, TShouldBe> : public TShouldBe {};
template <typename TShouldBe> struct ensured_uint_t<uint32_t, TShouldBe> : public TShouldBe {};

template <typename T> struct uint_larger_holder_t;
template <> struct uint_larger_holder_t<uint8_t>  { static uint16_t typedValue; };
template <> struct uint_larger_holder_t<uint16_t> { static uint32_t typedValue; };
template <> struct uint_larger_holder_t<uint32_t> { static uint64_t typedValue; };
template <typename T> using uint_larger_t = decltype(uint_larger_holder_t<T>::typedValue);

template <typename T> struct int_larger_holder_t;
template <> struct int_larger_holder_t<int8_t>  { static int16_t typedValue; };
template <> struct int_larger_holder_t<int16_t> { static int32_t typedValue; };
template <> struct int_larger_holder_t<int32_t> { static int64_t typedValue; };
template <typename T> using int_larger_t = decltype(int_larger_holder_t<T>::typedValue);

template <typename T> struct uint_smaller_holder_t;
template <> struct uint_smaller_holder_t<uint16_t> { static uint8_t  typedValue; };
template <> struct uint_smaller_holder_t<uint32_t> { static uint16_t typedValue; };
template <> struct uint_smaller_holder_t<uint64_t> { static uint32_t typedValue; };
template <typename T> using uint_smaller_t = decltype(uint_smaller_holder_t<T>::typedValue);

template <typename T> struct int_smaller_holder_t;
template <> struct int_smaller_holder_t<uint16_t> { static int8_t  typedValue; };
template <> struct int_smaller_holder_t<uint32_t> { static int16_t typedValue; };
template <> struct int_smaller_holder_t<uint64_t> { static int32_t typedValue; };
template <typename T> using int_smaller_t = decltype(int_smaller_holder_t<T>::typedValue);

template <typename T> struct int_similar_holder_t;
template <> struct int_similar_holder_t<uint8_t>  { static int8_t  typedValue; };
template <> struct int_similar_holder_t<uint16_t> { static int16_t typedValue; };
template <> struct int_similar_holder_t<uint32_t> { static int32_t typedValue; };
template <> struct int_similar_holder_t<uint64_t> { static int64_t typedValue; };
template <typename T> using int_similar_t = decltype(int_similar_holder_t<T>::typedValue);

template <typename T> struct uint_similar_holder_t;
template <> struct uint_similar_holder_t<int8_t>  { static uint8_t  typedValue; };
template <> struct uint_similar_holder_t<int16_t> { static uint16_t typedValue; };
template <> struct uint_similar_holder_t<int32_t> { static uint32_t typedValue; };
template <> struct uint_similar_holder_t<int64_t> { static uint64_t typedValue; };
template <typename T> using uint_similar_t = decltype(uint_similar_holder_t<T>::typedValue);