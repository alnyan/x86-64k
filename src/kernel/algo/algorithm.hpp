#pragma once

template<typename T> T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T> T min(T a, T b) {
    return a < b ? a : b;
}

#define alignUp(x, a) (((x) + ((a) - 1)) & -(a))
