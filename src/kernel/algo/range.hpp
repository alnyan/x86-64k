#pragma once
#include <algo/algorithm.hpp>

template<typename T> struct range_iterator {
    range_iterator(T val, T e, T st): value{val}, end{e}, step{st} {
    }

    T operator *() const {
        return value;
    }

    void operator ++() {
        value += step;
        if (value > end) {
            value = end;
        }
    }

    bool operator !=(const range_iterator<T> &other) const {
        return value != other.value;
    }

    bool operator ==(const range_iterator<T> &other) const {
        return value == other.value;
    }

    T value, end, step;
};

template<typename T> struct range_iterable {
    range_iterable(T s, T e, T st): _now{s}, _end{e}, _step{st} {
    }

    range_iterator<T> begin() const {
        return range_iterator<T>(_now, _end, _step);
    }

    range_iterator<T> end() const {
        return range_iterator<T>(_end, _end, _step);
    }

    T _now, _end, _step;
};

template<typename T> struct range {
    range(T s, T e): start{s}, end{e} {
    }

    range(): start{0}, end{0} {
    }

    bool contains(T v) const {
        return v >= start && v <= end;
    }

    range_iterable<T> iter(T step = 1) const {
        return range_iterable<T>(start, end, step);
    }

    T start, end;
};
