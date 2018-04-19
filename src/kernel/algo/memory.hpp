#pragma once
#include <algo/new.hpp>
#include <sys/types.h>

extern "C" {
    void *memcpy(void *dst, const void *src, size_t lim);
    void *memset(void *block, int value, size_t lim);
}

/**
 * \brief Unique pointer class, non-null
 */
template<typename T> class ptr {
public:
    /**
     * \brief Create unique ptr from raw pointer
     * \param p         - Pointer, non-null
     */
    ptr(T *p): m_value{p} {
        assert(p);
    }

    /**
     * \brief Deallocates value of owned pointer
     */
    ~ptr() {
        delete m_value;
    }

    /**
     * \brief Mutable dereferencing
     * \return Mutable reference to value the ptr points to
     */
    T &operator *() {
        return *m_value;
    }

    /**
     * \brief Immutable dereferencing
     * \return Constant reference to value the ptr points to
     */
    const T &operator *() const {
        return *m_value;
    }

    /**
     * \brief Mutable pointer member
     * \return Pointer
     */
    T *operator ->() {
        return m_value;
    }

    /**
     * \brief Immutable pointer member
     * \return Pointer
     */
    const T *operator ->() const {
        return m_value;
    }

    /**
     * \brief Mutable pointer
     * \return Pointer
     */
    T *get() {
        return m_value;
    }

    /**
     * \brief Immutable pointer
     * \return Pointer
     */
    const T *get() const {
        return m_value;
    }

private:

    T *m_value;
};
