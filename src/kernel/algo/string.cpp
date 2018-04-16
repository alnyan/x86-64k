#include "string.hpp"
#include <algo/algorithm.hpp>
#include <sys/debug.hpp>

str::str(const char *ptr): m_length{strlen(ptr)}, m_data{ptr} {
}

str::str(const char *ptr, size_t len): m_length{len}, m_data{ptr} {
}

str str::slice(size_t end) const {
    return str(m_data, min(end, m_length));
}

str str::slice(size_t begin, size_t end) const {
    uintptr_t start = min(begin, m_length);
    return str(m_data + start, min(max(begin, end), m_length) - start);
}

const char *str::data() const {
    return m_data;
}

size_t str::length() const {
    return m_length;
}

char str::operator [](size_t i) const {
    return m_data[i];
}

bool str::operator !=(const str &other) const {
    if (m_length != other.m_length) {
        return true;
    }
    return strncmp(m_data, other.m_data, m_length);
}

bool str::operator ==(const str &other) const {
    if (m_length != other.m_length) {
        return false;
    }
    return !strncmp(m_data, other.m_data, m_length);
}
