#pragma once
#include <sys/types.h>

extern "C" {
    size_t strlen(const char *s);
    const char *strcmp(const char *a, const char *b);
    const char *strncmp(const char *a, const char *b, size_t lim);
}

// Immutable static string
class str {
public:
    str();
    str(const char *ptr);
    str(const char *ptr, size_t len);

    str slice(size_t end) const;
    str slice(size_t start, size_t end) const;

    size_t length() const;
    const char *data() const;

    bool operator ==(const str &other) const;
    bool operator !=(const str &other) const;

    char operator [](size_t index) const;

private:

    size_t m_length;
    const char *m_data;
};
