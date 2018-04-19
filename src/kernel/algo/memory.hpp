#pragma once
#include <algo/new.hpp>
#include <sys/types.h>

extern "C" {
    void *memcpy(void *dst, const void *src, size_t lim);
    void *memset(void *block, int value, size_t lim);
}

template<typename T> class ptr {
public:
    ptr(T *p): m_value{p} {
        assert(p);
    }

    ~ptr() {
        delete m_value;
    }

    T &operator *() {
        return *m_value;
    }

    const T &operator *() const {
        return *m_value;
    }

    T *operator ->() {
        return m_value;
    }

    const T *operator ->() const {
        return m_value;
    }

    T *get() {
        return m_value;
    }

    const T *get() const {
        return m_value;
    }

private:

    T *m_value;
};
