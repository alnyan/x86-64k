#include <sys/debug.hpp>
#include <mem/heap.hpp>
#include <reent.h>

#define STUB(name) void name() { debug::dpanic(#name " is not implemented"); }
#define STUBD(name, type, ...) type name(__VA_ARGS__) { debug::dpanic(#name " is not implemented"); }

extern "C" {
    typedef int FILE;

    struct _reent _impure_data;
    struct _reent *_impure_ptr = &_impure_data;

    size_t fwrite ( const void * ptr, size_t size, size_t count, FILE * stream ) {
        debug::dprintf(static_cast<const char*>(ptr));
        return count;
    }
    void *malloc(size_t sz) { 
        debug::dprintf("malloc of size %d\n", sz);
        auto ptr = heap::kernelHeap.alloc(sz).orPanic("malloc failed");
        debug::dprintf("mallocated ptr %a\n", ptr);
        return ptr;
    }
    void free(void *ptr) { 
        debug::dprintf("dealloc of ptr %a\n", ptr);
        return heap::kernelHeap.free(ptr);
    }
    int fflush(FILE *stream) { return 0; }
    int putc(int chr, FILE *stream) { 
        char ch[2] = {static_cast<char>(chr), 0};
        debug::dputs(ch); 
        return chr;
    }

    STUB(abort)
    STUB(_ctype_)
    STUB(__errno)
    STUB(fclose)
    STUB(fdopen)
    STUB(fileno)
    STUB(fopen)
    STUB(fputc)
    STUB(fputs)
    STUB(fread)
    STUB(fseek)
    STUB(fstat)
    STUB(ftell)
    STUB(getc)
    STUB(getwc)
    STUB(iswctype)
    STUB(__locale_mb_cur_max)
    STUB(lseek)
    STUB(mbrtowc)
    STUB(putwc)
    STUB(read)
    STUB(realloc)
    STUB(setlocale)
    STUB(setvbuf)
    STUB(sprintf)
    STUB(sscanf)
    STUB(strftime)
    STUB(strtod)
    STUB(strtof)
    STUB(strtoul)
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