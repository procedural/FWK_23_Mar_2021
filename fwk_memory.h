// memory framework
// - rlyeh, public domain

#ifndef __linux__

#ifndef MEMORY_H
#define MEMORY_H
#include <stdlib.h>
#include <string.h>

#ifdef REALLOC
#error
#endif

// memory api
#define MSIZE(p)      xsize(p)
#define REALLOC(p,sz) (rllcsz_ = (sz), (rllcsz_ ? WATCH(xrealloc((p),rllcsz_),rllcsz_) : xrealloc(FORGET(p),0)))
#define MALLOC(sz)    REALLOC(0,(sz))
#define FREE(p)       REALLOC(FORGET(p), 0)
#define CALLOC(n,sz)  (rllcsz_ = (n)*(sz), memset(REALLOC(0,rllcsz_),0,rllcsz_))
#define STRDUP(s)     (strdsz_ = strlen(s)+1, ((char*)memcpy(REALLOC(0,strdsz_), (s), strdsz_)))
static threadlocal size_t rllcsz_, strdsz_;

// memory leaks detector
#if WITH_LEAK_DETECTOR
#define WATCH(ptr,sz) watch((ptr), (sz))
#define FORGET(ptr)   forget(ptr)
#else
#define WATCH(ptr,sz) (ptr)
#define FORGET(ptr)   (ptr)
#endif

// default allocator (aborts on out-of-mem)
void*  xrealloc(void* p, size_t sz);
size_t xsize(void* p);

// stack based allocator (negative bytes does rewind stack, like when entering new frame)
void*  stack(int bytes);

// memory leaks
void*  watch( void *ptr, int sz );
void*  forget( void *ptr );

#endif

// --------------------------------------------------------------------------

#ifdef MEMORY_C
#pragma once

#ifndef SYS_REALLOC
#define SYS_REALLOC realloc
#endif

#ifndef SYS_MSIZE
#include <stdint.h>
#if defined(__GLIBC__)
#  include <malloc.h>
#  define SYS_MSIZE malloc_usable_size
#elif defined(__APPLE__) || defined(__FreeBSD__)
#  include <malloc/malloc.h>
#  define SYS_MSIZE  malloc_size
#elif defined(__ANDROID_API__)
#  include <malloc.h>
/*extern "C"*/ size_t dlmalloc_usable_size(void*);
#  define SYS_MSIZE dlmalloc_usable_size
#elif defined(_WIN32)
#  include <malloc.h>
#  define SYS_MSIZE _msize
#else
#  error Unsupported malloc_usable_size()
#endif
#endif

// xrealloc --------------------------------------------------------------------

void* xrealloc(void* oldptr, size_t size) {
    void *ptr = SYS_REALLOC(oldptr, size);
    if( !ptr && size ) {
        PANIC("Not memory enough (trying to allocate %u bytes)", (unsigned)size);
    }
#ifdef XREALLOC_POISON
    if( !oldptr && size ) {
        memset(ptr, 0xCD, size); // test me
    }
#endif
    return ptr;
}
size_t xsize(void* p) {
    if( p ) return SYS_MSIZE(p);
    return 0;
}

// stack -----------------------------------------------------------------------

void* stack(int bytes) { // use negative bytes to rewind stack
    static threadlocal uint8_t *stack_mem = 0;
    static threadlocal uint64_t stack_ptr = 0;
    static threadlocal uint64_t stack_max = 0; // watch this var, in case you want to fine tune 4 MiB value below
    if( bytes < 0 ) {
        if( stack_ptr > stack_max ) stack_max = stack_ptr;
        return (stack_ptr = 0), NULL;
    }
    if( !stack_mem ) stack_mem = xrealloc(stack_mem, xsize(stack_mem) + 4 * 1024 * 1024);
    return &stack_mem[ (stack_ptr += bytes) - bytes ];
}

// leaks ----------------------------------------------------------------------

void* watch( void *ptr, int sz ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        for( FILE *fp = fopen(buf, "a+"); fp; fclose(fp), fp = 0 ) {
            fseek(fp, 0L, SEEK_END);
            char *callstack(int);
            const char *cs = callstack( +48 ); // NULL; // =
            fprintf(fp, "Built %s %s\n", __DATE__, __TIME__); // today() instead?
            fprintf(fp, "Memleak address: [%p], size: %d\n%s\n", ptr, sz, cs ? cs : "No callstack.");
        }
    }
    return ptr;
}
void* forget( void *ptr ) {
    if( ptr ) {
        char buf[256];
        sprintf(buf, "%p.mem", ptr);
        unlink(buf);
    }
    return ptr;
}

#endif

#endif // #ifndef __linux__