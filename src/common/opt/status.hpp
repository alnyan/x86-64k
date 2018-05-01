#pragma once
#include <sys/debug.hpp>

enum class status_kind_t {
    OK,
    ERROR,
    WTF
};

#define TO_STRING2(x) #x
#define TO_STRING(x) TO_STRING2(x)

#define STATUS_AUTO_OK(stat) (statuses::ok(stat, TO_STRING(stat)))
#define STATUS_AUTO_ERR(stat) (statuses::error(stat, TO_STRING(stat)))
#define STATUS_AUTO_WTF(stat) (statuses::wtf(stat, TO_STRING(stat)))

// todo put somewhere
static inline const char *strOfKind(status_kind_t kind) {
    switch (kind) {
        case status_kind_t::OK: return "OK";
        case status_kind_t::ERROR: return "ERROR";
        case status_kind_t::WTF: return "WTF";
        default: return "UNKNOWN";
    }
}

template <typename T>
class status {
private:
    T m_stat;
    status_kind_t m_kind;
    const char *m_descr;
public:
    status(T stat, status_kind_t kind, const char *descr)
        : m_stat(stat), m_kind(kind), m_descr(descr) {};

    T statusCode() const { return m_stat; }
    status_kind_t kind() const { return m_kind; }
    const char *description() const { return m_descr; }

    bool isError() const { return m_kind != status_kind_t::OK; }
    void panicOnError() const { 
        if (isError()) { 
            debug::dpanicf("status<T>: panicOnError() with kind: %s and status %d: %s\n", strOfKind(m_kind), m_stat, m_descr); 
        }
    }
};

class statuses {
public:
    template <typename T>
    static status<T> ok(T stat, const char *descr) { return status<T>(stat, status_kind_t::OK, descr); }

    template <typename T>
    static status<T> error(T stat, const char *descr) { return status<T>(stat, status_kind_t::ERROR, descr); }

    template <typename T>
    static status<T> wtf(T stat, const char *descr) { return status<T>(stat, status_kind_t::WTF, descr); }
};