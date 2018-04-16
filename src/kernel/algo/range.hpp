#pragma once

template<typename T> struct range {
    range(T s, T e): start{s}, end{e} {
    }

    range(): start{0}, end{0} {
    }

    bool contains(T v) const {
        return v >= start && v <= end;
    }

    T start, end;
};
