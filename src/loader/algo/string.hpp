#pragma once
#include <sys/types.h>

extern "C" {
    const char *strncmp(const char *a, const char *b, size_t l);
}

class string_slice;

class string_slice {
public:
    string_slice(const char *p, size_t lim)
        : m_ptr{p}, m_length{lim} {
    }

    string_slice(const unsigned char *p, size_t lim)
        : m_ptr{reinterpret_cast<const char *>(p)}, m_length{lim} {
    }

    const char *data() const {
        return m_ptr;
    }

    size_t length() const {
        return m_length;
    }

private:
    const char *m_ptr;
    size_t m_length;
};

template<size_t L> class str {
public:
    constexpr str(const char (&s)[L]): m_data{s} {

    }

    constexpr str(const unsigned char (&s)[L]): m_data{reinterpret_cast<const char *>(s)} {

    }

    constexpr size_t length() const {
        return L;
    }

    bool operator ==(const string_slice &slice) const {
        return !strncmp(m_data, slice.data(), slice.length());
    }

    string_slice slice(size_t end) const {
        return string_slice(m_data, end);
    }

private:
    const char *m_data;
};
