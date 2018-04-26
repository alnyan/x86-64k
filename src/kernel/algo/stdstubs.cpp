#include <sys/panic.hpp>

#define STUB(name) void name() { panic_msg(#name " is not implemented"); }
#define STUBD(name, type, ...) type name(__VA_ARGS__) { panic_msg(#name " is not implemented"); }

extern "C" {
    typedef int wint_t;
    typedef int wctype_t;
    typedef int FILE;

    extern const uint64_t _impure_ptr = 0x200000;

    int wctob(wint_t wc) { return wc; }
    wint_t btowc(int c) { return c; }
    wctype_t wctype(const char* prop) { return 0; }
    int strcmp(const char *a, const char *b, size_t lim) {
        size_t i = 0;
        while (i < lim && (*a || *b)) {
            if (*a != *b) {
                return 1;
            }
            ++a;
            ++b;
            ++i;
        }
        return 0;
    }
    size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ) {
        debug::printf(static_cast<const char*>(ptr));
        return size;
    }

    STUB(abort)
    STUB(_ctype_)
    STUB(__errno)
    STUB(fclose)
    STUB(fdopen)
    STUB(fflush)
    STUB(fileno)
    STUB(fopen)
    STUB(fputc)
    STUB(fputs)
    STUB(fread)
    STUB(free)
    STUB(fseek)
    STUB(fstat)
    STUB(ftell)
    STUB(getc)
    STUB(getwc)
    STUB(iswctype)
    STUB(__locale_mb_cur_max)
    STUB(lseek)
    STUB(malloc)
    STUB(mbrtowc)
    STUB(memchr)
    STUB(memcmp)
    STUB(putc)
    STUB(putwc)
    STUB(read)
    STUB(realloc)
    STUB(setlocale)
    STUB(setvbuf)
    STUB(sprintf)
    STUB(sscanf)
    STUB(strcoll)
    STUB(strerror)
    STUB(strftime)
    STUB(strtod)
    STUB(strtof)
    STUB(strtoul)
    STUB(strxfrm)
    STUB(towlower)
    STUB(towupper)
    STUB(ungetc)
    STUB(ungetwc)
    STUB(_Unwind_DeleteException)
    STUB(_Unwind_GetDataRelBase)
    STUB(_Unwind_GetIPInfo)
    STUB(_Unwind_GetLanguageSpecificData)
    STUB(_Unwind_GetRegionStart)
    STUB(_Unwind_GetTextRelBase)
    STUB(_Unwind_RaiseException)
    STUB(_Unwind_Resume)
    STUB(_Unwind_Resume_or_Rethrow)
    STUB(_Unwind_SetGR)
    STUB(_Unwind_SetIP)
    STUB(vsnprintf)
    STUB(wcrtomb)
    STUB(wcscoll)
    STUB(wcsftime)
    STUB(wcslen)
    STUB(wcsxfrm)
    STUB(wmemchr)
    STUB(wmemcmp)
    STUB(wmemcpy)
    STUB(wmemmove)
    STUB(wmemset)
    STUB(write)
}