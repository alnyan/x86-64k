#pragma once

// Various useful stuff here
namespace std {

    using nullptr_t = decltype(nullptr);

}

template<typename T> struct range {
    T start, end;

    bool contains(T v) const {
        return v >= start && v <= end;
    }

    T length() const {
        return end - start;
    }
};

template<typename A, A _start, A _end> struct const_range {
    static constexpr A start = _start, end = _end;

    bool contains(A v) const {
        return v >= start && v <= end;
    }

    A length() const {
        return end - start;
    }
};

// Placement "new" operator
void *operator new(unsigned long p, int v);
