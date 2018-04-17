#pragma once
#include <sys/panic.hpp>

inline void *operator new(size_t v, void *p) throw() {
    return p;
}

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
            panic_msg("Attempted to dereference empty option<T>\n");
        }
        return *m_ptr;
    }

private:
    uint8_t m_value[sizeof(T)] alignas(T);
    T *m_ptr;
};
