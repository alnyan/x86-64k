#ifndef NO_STDCXX

#include <cwchar>
#include <cwctype>
#include <cstring>
#include <cstddef>

#else

#include <nostdcxx/string.hpp>
#include <stddef.h>
using wint_t = int;
using wctype_t = int;

#endif

// no unicode support at this point
extern "C" int wctob(wint_t wc) { return wc; }
extern "C" wint_t btowc(int c) { return c; }

static const char* s_wctypes[] = 
    {"alnum", "alpha", "blank", "cntrl", 
     "digit", "graph", "lower", "print", 
     "punct", "space", "upper", "xdigit"};

extern "C" wctype_t wctype(const char* prop) {
    for (size_t  i = 0; i < sizeof(s_wctypes) / sizeof(char*); i++) {
        if (!strcmp(prop, s_wctypes[i])) return i;
    }
    return -1;
}