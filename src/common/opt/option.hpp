#pragma once
#include <sys/debug.hpp>

// костыль
#ifdef __LP64__ 
#include <new>
#include <cstdint>
#else
#include <stdint.h>
inline void *operator new(size_t v, void *p) throw() {
    return p;
}
#endif

template<typename T> class option {
public:
    option(): m_ptr{nullptr} {

    }

    option(const T &v): m_ptr(new (m_value) T(v)) {
    }

    static option<T> some(T v) {
        return option<T>(v);
    }

    static option<T> none() {
        return option<T>();
    }

    operator bool() const {
        return m_ptr != nullptr;
    }

    const T &operator *() const {
        if (!m_ptr) {
            debug::dpanic("Attempted to dereference empty option<T>\n");
        }
        return *m_ptr;
    }

    const T &orPanic(const char *msg) const {
        if (!m_ptr) {
            debug::dpanic(msg);
        }
        return *m_ptr;
    }

private:
    uint8_t m_value[sizeof(T)];
    T *m_ptr;
};
