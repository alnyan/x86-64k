#pragma once
#include <sys/panic.hpp>

inline void *operator new(size_t v, void *p) throw() {
    return p;
}

/**
 * \brief Optional container, may contain value or be empty
 */
template<typename T> class option {
public:
    /**
     * \brief Empty option constructor
     */
    option(): m_ptr{nullptr} {

    }

    /**
     * \brief Constructs option from a value
     */
    option(const T &v): m_ptr(new (m_value) T(v)) {
    }

    /**
     * \return true if option contains a value, false otherwise
     */
    operator bool() const {
        return m_ptr != nullptr;
    }

    /**
     * \brief Dereferences the option, raises panic if the option is emtpy
     * \return Const reference to the object held by option
     */
    const T &operator *() const {
        if (!m_ptr) {
            panic_msg("Attempted to dereference empty option<T>\n");
        }
        return *m_ptr;
    }

    /**
     * \brief Same as option<T>::operator *, but with custom message
     * \return Const reference to the object held by option
     */
    const T &orPanic(const char *msg) const {
        if (!m_ptr) {
            panic_msg(msg);
        }
        return *m_ptr;
    }

private:
    uint8_t m_value[sizeof(T)] alignas(T);
    T *m_ptr;
};
