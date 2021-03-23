// data structures and utils: array, map, set, string, hash, sort.
// - rlyeh, public domain

#ifdef DS_C
//#define ARRAY_C
#define ALLOC_C
#define HASH_C
#define MAP_C
#define STRING_C
#define LESS_C
#define SET_C
#endif

#ifndef DS_H
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#  if defined __ANDROID_API__
#define _AND
#elif defined __APPLE__
#define _OSX
#elif defined __FreeBSD__
#define _BSD
#elif defined _WIN32
#define _WIN
#else
#define _LIN
#endif

#  if defined _MSC_VER
#define _MSC
#elif defined __clang__
#define _CLA
#elif defined __TINYC__
#define _TCC
#else
#define _GCC
#endif

#ifdef  _MSC
#define THREAD __declspec(thread)
#else
#define THREAD __thread
#endif

#ifndef REALLOC
#define     REALLOC        realloc
#define MALLOC(n)          REALLOC(0, n)
#define FREE(p)            REALLOC(p, 0)
#define CALLOC(c, n)       memset(MALLOC((c)*(n)), 0, (c)*(n))
#define STRDUP(s)          strcpy(MALLOC(strlen(s)+1), (s))
#endif

#ifndef MSIZE
#if defined _OSX || defined _BSD
#    define MSIZE          malloc_size
#elif defined _AND
#    define MSIZE          dlmalloc_usable_size
#elif defined _WIN
#    define MSIZE          _msize
#else
#    define MSIZE          malloc_usable_size // glibc
#endif
#endif

#endif // DS_H

// string framework
// - rlyeh, public domain

#ifndef STRING_H
#define STRING_H

// string: temporary api (stack)
char*   stringf(const char *fmt, ...);
#define stringf(...) (printf || printf(__VA_ARGS__), stringf(__VA_ARGS__))  // vs2015 check trick

// string: allocated api (heap)
char*   stringf_cat(char *x, const char *buf);
#define stringf_cat(s,fmt,...) stringf_cat((s), stringf(fmt, __VA_ARGS__)) // stringfcat ?
#define stringf_del(s)         ((is_stringf(s) ? (void)0 : REALLOC((s), 0)), (s)=0) // stringfdel ?

// string: utils
int     is_stringf(const char *s);

#ifdef _MSC_VER
#define strtok_r strtok_s
#endif

#define each_substring(str, delims, keyname) \
    ( char buf[1024], *_lit = (char*)(str), *_bak = (snprintf(buf, 1024, "%s", _lit), _lit); _bak; _bak = 0 ) \
    for( char *next_token = 0, *keyname = strtok_r(_bak, delims, &next_token); keyname; keyname = strtok_r(NULL, delims, &next_token) )

#endif // STRING_H

// -----------------------------------------------------------------------------

#ifdef STRING_C
#pragma once

int is_stringf(const char *s) {
    return (1&(uintptr_t)s) && s[-1] == 0;
}
char* stringfv(const char *fmt, va_list vl) {
    va_list copy;
    va_copy(copy, vl);
    int sz = vsnprintf( 0, 0, fmt, copy ) + 1;
    va_end(copy);

    int reqlen = sz + 1; // 1 for even padding

    char* ptr;
    enum { STACK_ALLOC = 16384 };
    if( reqlen < STACK_ALLOC ) { // fit stack?
        static THREAD char buf[STACK_ALLOC+1];
        static THREAD int cur = 1, len = STACK_ALLOC;
        ptr = buf + ((cur+reqlen) > len ? cur = 1 : (cur += reqlen) - reqlen);
    } else { // else use heap (@fixme: single memleak per invoking thread)
        static THREAD char *buf = 0;
        static THREAD int cur = 1, len = -STACK_ALLOC; // unsigned?
abort(); // test me
        if( reqlen >= len ) buf = (char*)REALLOC(buf, len = abs(len) * 1.75 + reqlen); // remove abs?
        ptr = buf + ((cur+reqlen) > len ? cur = 1 : (cur += reqlen) - reqlen);
    }

    ptr += !(1&(uintptr_t)ptr); // align to even address only when already odd
    ptr[-1] = 0; // add header
    assert(is_stringf(ptr));

    vsnprintf( ptr, sz, fmt, vl );
    return (char *)ptr;
}
char* (stringf)(const char *fmt, ...) {
    va_list va;
    va_start(va, fmt);
    char *s = stringfv(fmt, va);
    va_end(va);
    return s;
}
char* (stringf_cat)(char *src, const char *buf) {
    int srclen = (src ? strlen(src) : 0), buflen = strlen(buf);
    src = (char*)REALLOC(src, srclen + buflen + 1 );
    memcpy(src + srclen, buf, buflen + 1 );
    return src;
}

#if 0
    #define stringf_foreach_i(s, chars) stringf_foreach(i,s,chars)
    #define stringf_foreach_j(s, chars) stringf_foreach(j,s,chars)
    #define stringf_foreach_k(s, chars) stringf_foreach(k,s,chars)
    #define stringf_foreach(iter, s, chars) \
        for( char *iter = is_stringf(s) ? strtok(s, chars) : NULL; iter; iter = strtok(NULL, chars) )

    char *str = stringf("hello, world! it's a trap :) %d!",123);
    stringf_foreach_i(str, " ,:)'!") {
        puts(i);
    }
#endif

#endif // STRING_C

#ifndef LESS_H
#define LESS_H

int less_int(int a, int b);
int less_u64(uint64_t a, uint64_t b);
int less_ptr(void *a, void *b);
int less_str(char *a, char *b); // non-const args for C++

#endif // LESS_H

#ifdef LESS_C
#pragma once

int less_int(int a, int b) {
    return a - b;
}
int less_u64(uint64_t a, uint64_t b) {
    return a > b ? +1 : -!!(a - b);
}
int less_ptr(void *a, void *b) {
    return (uintptr_t)a > (uintptr_t)b ? +1 : -!!((uintptr_t)a - (uintptr_t)b);
}
int less_str(char *a, char *b) { // non-const args for C++
#if 0 // useful?
    int sa = strlen((const char*)a);
    int sb = strlen((const char*)b);
    return sa>sb ? +1 : sa<sb ? -1 : strncmp((const char*)a, (const char*)b, sa);
#else
    return strcmp((const char *)a, (const char *)b);
#endif
}

#endif // LESS_C

#ifndef HASH_H
#define HASH_H

uint64_t hash_int(int key);
uint32_t hash_32(uint32_t x);
uint64_t hash_64(uint64_t x);
uint64_t hash_flt(double x);
uint64_t hash_str(char* str); // non-const arg for C++
uint64_t hash_ptr(const void *ptr);

#endif // HASH_H

#ifdef HASH_C
#pragma once

// { Thomas Mueller at https://stackoverflow.com/questions/664014/ - says no collisions for 32bits!

uint32_t hash_32(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

uint32_t unhash_32(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = ((x >> 16) ^ x) * 0x119de1f3;
    x = (x >> 16) ^ x;
    return x;
}

uint64_t hash_64(uint64_t x) {
    x = (x ^ (x >> 30)) * UINT64_C(0xbf58476d1ce4e5b9);
    x = (x ^ (x >> 27)) * UINT64_C(0x94d049bb133111eb);
    x = x ^ (x >> 31);
    return x;
}

uint64_t hash_flt(double x) {
    union { double d; uint64_t i; } c; c.d = x;
    return hash_64(c.i);
}

// }

uint64_t hash_str(char* str) { // non-const arg for C++
    // faster than fnv1a, a few more collisions though.
    uint64_t hash = 0; // fnv1a: 14695981039346656037ULL;
    while( *str ) {
        hash = ( (unsigned char)*str++ ^ hash ) * 131; // fnv1a: 0x100000001b3ULL;
    }
    return hash;
}
uint64_t hash_int(int key) {
    // return hash_64((uint64_t)key);
    // return hash_mix64((uint64_t)key);
    // return hash_triple32((uint32_t)key);
    return hash_32((uint32_t)key);
}
uint64_t hash_ptr(const void *ptr) {
    uint64_t key = (uint64_t)(uintptr_t)ptr;
    return hash_64(key >> 3); // >> 4? needed?
}

#endif // HASH_C

// #include "ds_hash.c"
// #include "ds_sort.c"

// [ ] @todo: gc, pool, game mark/rewind+game stack |level|game|>---<stack|
// - rlyeh, public domain

#ifndef ALLOC_H
#define ALLOC_H

// vector based allocator (x1.75 enlarge factor)
void*  vrealloc(void* p, size_t sz);
size_t vlen(void* p);

#endif // ALLOC_H

// vector ----------------------------------------------------------------------

#ifdef ALLOC_C
#pragma once

void* vrealloc( void* p, size_t sz ) {
    if( !sz ) {
        if( p ) {
            size_t *ret = (size_t*)p - 2;
            ret[0] = 0;
            ret[1] = 0;
            REALLOC( ret, 0 );
        }
        return 0;
    } else {
        size_t *ret = (size_t*)p - 2;
        if( !p ) {
            ret = (size_t*)REALLOC( 0, sizeof(size_t) * 2 + sz );
            ret[0] = sz;
            ret[1] = 0;
        } else {
            size_t osz = ret[0];
            size_t ocp = ret[1];
            if( sz <= (osz + ocp) ) {
                ret[0] = sz;
                ret[1] = ocp - (sz - osz);
            } else {
                ret = (size_t*)REALLOC( ret, sizeof(size_t) * 2 + sz * 1.75 );
                ret[0] = sz;
                ret[1] = (size_t)(sz * 1.75) - sz;
            }
        }
        return &ret[2];
    }
}
size_t vlen( void* p ) {
    return p ? 0[ (size_t*)p - 2 ] : 0;
}

#endif // ALLOC_C

// array library --------------------------------------------------------------
// - rlyeh, public domain

#ifndef ARRAY_H
#define ARRAY_H

#ifdef __cplusplus
#define array_cast(x) (decltype x)
#else
#define array_cast(x) (void *)
#endif

#define array(t) t*
#define array_init(t) ( (t) = 0 )
#define array_resize(t, n) ( array_c_ = array_count(t), array_realloc_((t),(n)), ((n)>array_c_? memset(array_c_+(t),0,((n)-array_c_)*sizeof(0[t])) : (void*)0), (t) )
#define array_push(t, ...) ( array_realloc_((t),array_count(t)+1), (t)[ array_count(t) - 1 ] = (__VA_ARGS__) )
#define array_pop(t) ( array_realloc_((t), array_count(t)-1) )
#define array_back(t) ( &(t)[ array_count(t)-1 ] ) // ( (t) ? &(t)[ array_count(t)-1 ] : NULL )
#define array_data(t) (t)
#define array_at(t,i) (t[i])
#define array_count(t) (int)( (t) ? array_vlen_(t) / sizeof(0[t]) : 0u )
#define array_bytes(t) (int)( (t) ? array_vlen_(t) : 0u )
#define array_sort(t, cmpfunc) qsort( t, array_count(t), sizeof(0[t]), cmpfunc )
#define array_empty(t) ( !array_count(t) )
static THREAD unsigned array_c_;

#if 0 // original: no reserve support
#define array_reserve(t, n) ((void)0) // not implemented
#define array_clear(t) ( array_realloc_((t), 0), (t) = 0 )
#define array_vlen_(t)  ( vlen(t) - 0 )
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+0) * sizeof(0[t])) )
#define array_free(t) array_clear(t)
#else // new: with reserve support (buggy still?)
#define array_reserve(t, n) ( array_realloc_((t),(n)), array_clear(t) )
#define array_clear(t) ( array_realloc_((t),0) ) // -1
#define array_vlen_(t)  ( vlen(t) - sizeof(0[t]) ) // -1
#define array_realloc_(t,n)  ( (t) = array_cast(t) vrealloc((t), ((n)+1) * sizeof(0[t])) ) // +1
#define array_free(t) ( array_realloc_((t), -1), (t) = 0 ) // -1
#endif

#define array_reverse(t) \
    do if( array_count(t) ) { \
        for(int l = array_count(t), e = l-1, i = (array_push(t, 0[t]), 0); i <= e/2; ++i ) \
            { l[t] = i[t]; i[t] = (e-i)[t]; (e-i)[t] = l[t]; } \
        array_pop(t); \
    } while(0)

//#define array_foreach2(t,val_t,v) \
//    for( val_t *v = &0[t]; v < (&0[t] + array_count(t)); ++v )

//#define array_foreach(t, it) \
//    for( void *end__ = (it = &0[t]) + array_count(t); it != end__; ++it )

#define array_foreach(t,val_t,v) for each_array(t,val_t,v)
#define each_array(t,val_t,v) \
    ( val_t *it__ = &0[t]; it__ < (&0[t] + array_count(t)); ++it__ ) \
        for( val_t v = *it__, *on__ = &v; on__; on__ = 0 )

#define array_foreach_ptr(t,val_t,v) for each_array_ptr(t,val_t,v)
#define each_array_ptr(t,val_t,v) \
    ( val_t *it__ = &0[t]; it__ < (&0[t] + array_count(t)); ++it__ ) \
        for( val_t *v = it__; v; v = 0 )

#define array_search(t, key, cmpfn) /* requires sorted array beforehand */ \
    bsearch(&key, t, array_count(t), sizeof(t[0]), cmpfn )

#define array_insert(t, i, n) do { \
    int ac = array_count(t); \
    if( i >= ac ) { \
        array_push(t, n); \
    } else { \
        array_push(t, array_back(t)); \
        memmove( &(t)[(i)+1], &(t)[i], (ac - (i)) * sizeof(t[0]) ); \
        (t)[ i ] = (n); \
    } \
} while(0)

#define array_copy(t, src) do { /*todo: review old vrealloc call!*/ \
    array_free(t); \
    (t) = vrealloc( (t), array_count(src) * sizeof(0[t])); \
    memcpy( (t), src, array_count(src) * sizeof(0[t])); \
} while(0)

#define array_erase(t, i) do { \
    memcpy( &(t)[i], &(t)[array_count(t) - 1], sizeof(0[t])); \
    array_pop(t); \
} while(0)

#define array_unique(t, cmpfunc) do { /*todo: review old vrealloc call!*/ \
    int cnt = array_count(t), dupes = 0; \
    if( cnt > 1 ) { \
        const void *prev = &(t)[0]; \
        array_sort(t, cmpfunc); \
        for( int i = 1; i < cnt; ) { \
            if( cmpfunc(&t[i], prev) == 0 ) { \
                memmove( &t[i], &t[i+1], (cnt - 1 - i) * sizeof(t[0]) ) ; \
                --cnt; \
                ++dupes; \
            } else { \
                prev = &(t)[i]; \
                ++i; \
            } \
        } \
        if( dupes ) { \
            (t) = vrealloc((t), (array_count(t) - dupes) * sizeof(0[t])); \
        } \
    } \
} while(0)

#endif // ARRAY_H

// generic map<K,V> container.
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/
// - rlyeh, public domain.

#ifndef MAP_H
#define MAP_H

#include <stdint.h>
#include <stdbool.h>

// config
#ifndef MAP_REALLOC
#define MAP_REALLOC REALLOC
#endif
#ifndef MAP_HASHSIZE
#define MAP_HASHSIZE (4096 << 4)
#endif
#ifndef MAP_DONT_ERASE
#define MAP_DONT_ERASE 1
#endif

// public api
#define map(K,V) \
    struct { map base; struct { pair p; K key; V val; } tmp, *ptr; V* tmpval; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define map_init(m, cmpfn, hashfn) ( \
    (m) = map_cast(m) MAP_REALLOC(0, sizeof(*(m))), \
    map_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = map_cast((m)->typed_cmp) cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = map_cast((m)->typed_hash) hashfn ) \
    )

#define map_free(m) ( \
    map_free(&(m)->base), \
    map_cast(m) MAP_REALLOC((m), sizeof(*(m))), (m) = 0 \
    )

#define map_insert(m, k, v) ( \
    (m)->ptr = map_cast((m)->ptr) MAP_REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->val = (v), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, &(m)->ptr->val, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->val \
    )

#define map_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = map_cast((m)->ptr) map_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->val : 0 \
    )

#define map_find_or_add(m, k, v) ( \
    (m)->tmp.key = (k), (m)->tmp.val = (v), \
    (m)->tmpval = map_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : map_insert((m), ((m)->tmp.key), ((m)->tmp.val)) \
    )

#define map_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    map_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define map_foreach(m,key_t,k,val_t,v) for each_map(m,key_t,k,val_t,v)
#define each_map(m,key_t,k,val_t,v) \
    ( int i_ = 0; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; ) \
                for( val_t v = *(val_t *)cur_->value; on_; on_ = 0 )

#define map_foreach_ptr(m,key_t,k,val_t,v) for each_map_ptr(m,key_t,k,val_t,v)
#define each_map_ptr(m,key_t,k,val_t,v) \
    ( int i_ = 0; i_ < MAP_HASHSIZE; ++i_) \
        for( pair *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; ) \
                for( val_t *v = (val_t *)cur_->value; on_; on_ = 0 )

#define map_clear(m) ( \
    map_clear(&(m)->base) \
    )

#define map_count(m)        map_count(&(m)->base)
#define map_gc(m)           map_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define map_cast(t) (decltype(t))
#else
#define map_cast(t) (void *)
#endif

typedef struct pair {
    struct pair *next;

    uint64_t keyhash;
    void *key;
    void *value;
    void *super;
} pair;

typedef struct map {
    array(pair*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count;
} map;

void  (map_init)(map *m);
void  (map_free)(map *m);

void  (map_insert)(map *m, pair *p, void *key, void *value, uint64_t keyhash, void *super);
void  (map_erase)(map *m, void *key, uint64_t keyhash);
void* (map_find)(map *m, void *key, uint64_t keyhash);
int   (map_count)(map *m);
void  (map_gc)(map *m); // only if using MAP_DONT_ERASE

#endif // MAP_H

// -------------------------------

#if defined MAP_C || defined MAP_DEMO
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum { MAP_GC_SLOT = MAP_HASHSIZE };
typedef int map_is_pow2_assert[ !(MAP_HASHSIZE & (MAP_HASHSIZE - 1)) ];

static int map_get_index(uint64_t hkey1) {
    return hkey1 & (MAP_HASHSIZE-1);
}

void (map_init)(map* m) {
    map c = {0};
    *m = c;

    array_resize(m->array, (MAP_HASHSIZE+1));
    memset(m->array, 0, (MAP_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (map_insert)(map* m, pair *p, void *key, void *value, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->value = value;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = map_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;

    ++m->count;
}

void* (map_find)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *cur = m->array[index]; cur; cur = cur->next ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                return cur->super;
            }
        }
    }
    return 0;
}

void (map_erase)(map* m, void *key, uint64_t keyhash) {
    int index = map_get_index(keyhash);
    for( pair *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if( prev ) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if MAP_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[MAP_GC_SLOT];
                m->array[MAP_GC_SLOT] = cur;
#else
                MAP_REALLOC(cur,0);
#endif
                --m->count;
                return;
            }
        }
    }
}

int (map_count)(map* m) {
    return m->count;
    int counter = 0;
    for( int i = 0; i < MAP_HASHSIZE; ++i) {
        for( pair *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

void (map_gc)(map* m) {
#if MAP_DONT_ERASE
    for( pair *next, *cur = m->array[MAP_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        MAP_REALLOC(cur,0);
    }
    m->array[MAP_GC_SLOT] = 0;
#endif
}

void (map_clear)(map* m) {
    for( int i = 0; i <= MAP_HASHSIZE; ++i) {
        for( pair *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            MAP_REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
}

void (map_free)(map* m) {
    (map_clear)(m);

    array_free(m->array);
    m->array = 0;

    map c = {0};
    *m = c;
}

// -------------------------------

#ifdef MAP_DEMO
#include <stdio.h>
#include <time.h>
#ifdef __cplusplus
#include <unordered_map>
//#include <map>
#endif

void map_benchmark() {
    #ifndef M
    #define M 100
    #endif
    #ifndef N
    #define N 50000
    #endif
    #define BENCH(CREATE,COUNT,INSERT,FIND,ITERATE,ERASE,DESTROY) do { \
        static char **bufs = 0; \
        if(!bufs) { \
            bufs = (char **)MAP_REALLOC(0, sizeof(char*) * N ); \
            for( int i = 0; i < N; ++i ) { \
                bufs[i] = (char*)MAP_REALLOC(0, 16); \
                sprintf(bufs[i], "%d", i); \
            } \
        } \
        clock_t t0 = clock(); \
        for( int i = 0; i < M; ++i ) { \
            CREATE; \
            if(i==0) printf("CREATE:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                INSERT; \
            } \
            if(i==0) printf("INSERT:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                FIND; \
            } \
            if(i==0) printf("FIND:%d ", (int)COUNT), fflush(stdout); \
            ITERATE; \
            if(i==0) printf("ITERATE:%d ", (int)COUNT), fflush(stdout); \
            for( int j = 0; j < N; ++j ) { \
                char *buf = bufs[j]; \
                ERASE; \
            } \
            if(i==0) printf("REMOVE:%d ", (int)COUNT), fflush(stdout); \
            DESTROY; \
            if(i==0) printf("DESTROY%s", " "); \
        } \
        clock_t t1 = clock(); \
        double t = (t1 - t0) / (double)CLOCKS_PER_SEC; \
        int ops = (M*N)*6; \
        printf("%d ops in %fs = %.2fM ops/s (" #CREATE ")\n", ops, t, ops / (t * 1e6)); \
    } while(0)

    map(char*,int) m = 0;
    BENCH(
        map_init(m, less_str, hash_str),
        map_count(m),
        map_insert(m, buf, i),
        map_find(m, buf),
        map_foreach(m,char*,k,int,v) {},
        map_erase(m, buf),
        map_free(m)
    );

#ifdef __cplusplus
    using std_unordered_map = std::unordered_map<const char *,int>;
    BENCH(
        std_unordered_map v,
        v.size(),
        v.insert(std::make_pair(buf, i)),
        v.find(buf),
        for( auto &kv : v ) {},
        v.erase(buf),
        {}
    );
#endif
#undef N
#undef M
}

void map_tests() {
    {
        map(int,char*) m = 0;
        map_init(m, less_int, hash_int);
            assert( 0 == map_count(m) );
        map_insert(m, 123, "123");
        map_insert(m, 456, "456");
            assert( 2 == map_count(m) );
            assert( map_find(m, 123) );
            assert( map_find(m, 456) );
            assert(!map_find(m, 789) );
            assert( 0 == strcmp("123", *map_find(m, 123)) );
            assert( 0 == strcmp("456", *map_find(m, 456)) );

        map_foreach(m,const int,k,char*,v) {
            printf("%d->%s\n", k, v);
        }

        map_erase(m, 123);
            assert(!map_find(m, 123) );
            assert( 1 == map_count(m) );
        map_erase(m, 456);
            assert(!map_find(m, 456) );
            assert( 0 == map_count(m) );
        map_free(m);
    }

    {
        map(char*,int) m = 0;
        map_init(m, less_str, hash_str);
            assert( map_count(m) == 0 );
        map_insert(m, "123", 123);
        map_insert(m, "456", 456);
            assert( map_count(m) == 2 );
            assert( map_find(m,"123") );
            assert( map_find(m,"456") );
            assert(!map_find(m,"789") );

        map_foreach(m,const char *,k,int,v) {
            printf("%s->%d\n", k, v);
        }

        map_erase(m, "123");
            assert( 456 == *map_find(m,"456") );
            assert( map_count(m) == 1 );
        map_erase(m, "456");
            assert( map_count(m) == 0 );
        map_free(m);

        assert(!puts("Ok"));
    }
}

void map_tests2() {
    map(char*,double) m = 0;

    map_init(m, less_str, hash_str);

    map_insert(m, "hello", 101.1);
    map_insert(m, "world", 102.2);
    map_insert(m, "nice", 103.3);
    map_insert(m, "hash", 104.4);

    assert(!map_find(m, "random"));

    assert(map_find(m, "hello"));
    assert(map_find(m, "world"));
    assert(map_find(m, "nice") );
    assert(map_find(m, "hash") );

    assert( 101.1 == *map_find(m, "hello"));
    assert( 102.2 == *map_find(m, "world"));
    assert( 103.3 == *map_find(m, "nice"));
    assert( 104.4 == *map_find(m, "hash"));

    // reinsertion
    assert(map_insert(m, "hello", -101.1));
    assert(-101.1 == *map_find(m, "hello"));

    map_foreach(m,char*,k,double,v) {
        printf("%s -> %f\n", k, v);
    }

    map_free(m);

    assert( !puts("Ok") );
}

void map_benchmark2() {
    #ifndef NUM
    #define NUM 2000000
    #endif

    map(int,int) m = 0;
    map_init(m, less_int, hash_int);

    clock_t t0 = clock();

    for( int i = 0; i < NUM; ++i ) {
        map_insert(m, i, i+1);
    }
    for( int i = 0; i < NUM; ++i ) {
        uint32_t *v = (uint32_t*)map_find(m, i);
        assert( v && *v == i + 1 );
    }

    double t = (clock() - t0) / (double)CLOCKS_PER_SEC;

    printf("[0]=%d\n", *map_find(m, 0));
    printf("[N-1]=%d\n", *map_find(m, NUM-1));
    printf("%d ops in %5.3fs = %fM ops/s\n", (NUM*2), t, (NUM*2) / (1e6 * t) );

    map_free(m);
}

int main() {
    map_tests();
    puts("---");
    map_tests2();
    puts("---");
    map_benchmark();
    puts("---");
    map_benchmark2();
    assert(~puts("Ok"));
}

#define main main__
#endif // MAP_DEMO
#endif // MAP_C

// generic set<K> container.
// ideas from: https://en.wikipedia.org/wiki/Hash_table
// ideas from: https://probablydance.com/2017/02/26/i-wrote-the-fastest-hashtable/
// ideas from: http://www.idryman.org/blog/2017/05/03/writing-a-damn-fast-hash-table-with-tiny-memory-footprints/
// - rlyeh, public domain.

#ifndef SET_H
#define SET_H

#include <stdint.h>
#include <stdbool.h>

// config
#ifndef SET_REALLOC
#define SET_REALLOC REALLOC
#endif
#ifndef SET_HASHSIZE
#define SET_HASHSIZE (4096 << 4)
#endif
#ifndef SET_DONT_ERASE
#define SET_DONT_ERASE 1
#endif

// public api
#define set(K) \
    struct { set base; struct { set_item p; K key; } tmp, *ptr; \
        int (*typed_cmp)(K, K); uint64_t (*typed_hash)(K); } *

#define set_init(m, cmpfn, hashfn) ( \
    (m) = set_cast(m) SET_REALLOC(0, sizeof(*m)), \
    set_init(&(m)->base), \
    (m)->base.cmp = (int(*)(void*,void*))( (m)->typed_cmp = cmpfn), \
    (m)->base.hash = (uint64_t(*)(void*))( (m)->typed_hash = hashfn ) \
    )

#define set_free(m) ( \
    set_clear(m), \
    set_free(&(m)->base), \
    (m) = set_cast(m) SET_REALLOC((m), 0), \
    (m) = 0 \
    )

#define set_insert(m, k) ( \
    (m)->ptr = set_cast((m)->ptr) SET_REALLOC(0, sizeof((m)->tmp)), \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_insert(&(m)->base, &(m)->ptr->p, &(m)->ptr->key, (m)->ptr->p.keyhash, (m)->ptr), \
    &(m)->ptr->key \
    )

#define set_find(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    (m)->ptr = set_cast((m)->ptr) set_find(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash), \
    (m)->ptr ? &(m)->ptr->key : 0 \
    )

#define set_find_or_add(m, k) ( \
    (m)->tmp.key = (k), \
    (m)->tmpval = set_find((m), ((m)->tmp.key)), \
    (m)->tmpval = (m)->tmpval ? (m)->tmpval : set_insert((m), ((m)->tmp.key)) \
    )

#define set_erase(m, k) ( \
    (m)->ptr = &(m)->tmp, \
    (m)->ptr->p.keyhash = (m)->typed_hash((m)->ptr->key = (k)), \
    set_erase(&(m)->base, &(m)->ptr->key, (m)->ptr->p.keyhash) \
    )

#define set_foreach(m,key_t,k) for each_set(m,key_t,k)
#define each_set(m,key_t,k) \
    ( int i_ = 0; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t k = *(key_t *)cur_->key; on_; on_ = 0 )

#define set_foreach_ptr(m,key_t,k) for each_set_ptr(m,key_t,k)
#define each_set_ptr(m,key_t,k) \
    ( int i_ = 0; i_ < SET_HASHSIZE; ++i_) \
        for( set_item *cur_ = (m)->base.array[i_], *on_ = cur_; cur_; on_ = cur_ = cur_->next ) \
            for( key_t *k = (key_t *)cur_->key; on_; on_ = 0 )

#define set_clear(m) ( \
    set_clear(&(m)->base) \
    )

#define set_count(m)        set_count(&(m)->base)
#define set_gc(m)           set_gc(&(m)->base)

// private:

#ifdef __cplusplus
#define set_cast(t) (decltype(t))
#else
#define set_cast(t) (void *)
#endif

typedef struct set_item {
    struct set_item *next;

    uint64_t keyhash;
    void *key;
    void *super;
} set_item;

typedef struct set {
    array(set_item*) array;
    int (*cmp)(void *, void *);
    uint64_t (*hash)(void *);
    int count;
} set;

void  (set_init)(set *m);
void  (set_free)(set *m);

void  (set_insert)(set *m, set_item *p, void *key, uint64_t keyhash, void *super);
void  (set_erase)(set *m, void *key, uint64_t keyhash);
void* (set_find)(const set *m, void *key, uint64_t keyhash);
int   (set_count)(const set *m);
void  (set_gc)(set *m); // only if using SET_DONT_ERASE

#endif // SET_H

// -------------------------------

#ifdef SET_C
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

enum { set_GC_SLOT = SET_HASHSIZE };
typedef int set_is_pow2_assert[ !(SET_HASHSIZE & (SET_HASHSIZE - 1)) ];

static int set_get_index(uint64_t hkey1) {
    return hkey1 & (SET_HASHSIZE-1);
}

void (set_init)(set* m) {
    set zero = {0};
    *m = zero;

    array_resize(m->array, (SET_HASHSIZE+1));
    memset(m->array, 0, (SET_HASHSIZE+1) * sizeof(m->array[0]) ); // array_resize() just did memset()
}

void (set_insert)(set* m, set_item *p, void *key, uint64_t keyhash, void *super) {
    p->keyhash = keyhash;
    p->key = key;
    p->super = super;

    /* Insert onto the beginning of the list */
    int index = set_get_index(p->keyhash);
    p->next = m->array[index];
    m->array[index] = p;

    ++m->count;
}

void* (set_find)(const set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( const set_item *cur = m->array[index]; cur; cur = cur->next ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                return cur->super;
            }
        }
    }
    return 0;
}

void (set_erase)(set* m, void *key, uint64_t keyhash) {
    int index = set_get_index(keyhash);
    for( set_item *prev = 0, *cur = m->array[index]; cur; (prev = cur), (cur = cur->next) ) {
        if( cur->keyhash == keyhash ) {
            char **c = (char **)cur->key;
            char **k = (char **)key;
            if( !m->cmp(c[0], k[0]) ) {
                if (prev) prev->next = cur->next; else m->array[index] = cur->next ? cur->next : 0;
#if SET_DONT_ERASE
                /* Insert onto the beginning of the GC list */
                cur->next = m->array[set_GC_SLOT];
                m->array[set_GC_SLOT] = cur;
#else
                SET_REALLOC(cur,0);
#endif
                --m->count;
                return;
            }
        }
    }
}

int (set_count)(const set* m) { // does not include GC_SLOT
    return m->count;
    int counter = 0;
    for( int i = 0; i < SET_HASHSIZE; ++i) {
        for( const set_item *cur = m->array[i]; cur; cur = cur->next ) {
            ++counter;
        }
    }
    return counter;
}

void (set_gc)(set* m) { // clean deferred GC_SLOT only
#if SET_DONT_ERASE
    for( set_item *next, *cur = m->array[set_GC_SLOT]; cur; cur = next ) {
        next = cur->next;
        SET_REALLOC(cur,0);
    }
    m->array[set_GC_SLOT] = 0;
#endif
}

void (set_clear)(set* m) { // include GC_SLOT
    for( int i = 0; i <= SET_HASHSIZE; ++i) {
        for( set_item *next, *cur = m->array[i]; cur; cur = next ) {
            next = cur->next;
            SET_REALLOC(cur,0);
        }
        m->array[i] = 0;
    }
    m->count = 0;
}

void (set_free)(set* m) {
    (set_clear)(m);

    array_free(m->array);
    m->array = 0;

    set zero = {0};
    *m = zero;
}

// -------------------------------

#ifdef SET_DEMO
#include <stdio.h>
#include <time.h>

void set_tests() {
    {
        set(int) m = {0};
        set_init(m, less_int, hash_int);
            assert( 0 == set_count(m) );
        set_insert(m, 123);
        set_insert(m, 456);
            assert( 2 == set_count(m) );
            assert( set_find(m, 123) );
            assert( set_find(m, 456) );
            assert(!set_find(m, 789) );
            assert( 123 == *set_find(m, 123) );
            assert( 456 == *set_find(m, 456) );

        set_foreach(m,const int,k) {
            printf("%d\n", k);
        }

        set_erase(m, 123);
            assert(!set_find(m, 123) );
            assert( 1 == set_count(m) );
        set_erase(m, 456);
            assert(!set_find(m, 456) );
            assert( 0 == set_count(m) );
        set_free(m);
    }

    {
        set(char*) m = {0};
        set_init(m, less_str, hash_str);
            assert( set_count(m) == 0 );
        set_insert(m, "123");
        set_insert(m, "456");
            assert( set_count(m) == 2 );
            assert( set_find(m,"123") );
            assert( set_find(m,"456") );
            assert(!set_find(m,"789") );

        set_foreach(m,const char *,k) {
            printf("%s\n", k);
        }

        set_erase(m, "123");
            assert( 0 == strcmp("456", *set_find(m,"456")) );
            assert( set_count(m) == 1 );
        set_erase(m, "456");
            assert( set_count(m) == 0 );
        set_free(m);

        assert(!puts("Ok"));
    }
}

void set_tests2() {
    set(char*) m;

    set_init(m, less_str, hash_str);

    set_insert(m, "hello");
    set_insert(m, "world");
    set_insert(m, "nice");
    set_insert(m, "hash");

    assert(!set_find(m, "random"));

    assert(set_find(m, "hello"));
    assert(set_find(m, "world"));
    assert(set_find(m, "nice"));
    assert(set_find(m, "hash"));

    assert( 0 == strcmp("hello", *set_find(m, "hello")));
    assert( 0 == strcmp("world", *set_find(m, "world")));
    assert( 0 == strcmp("nice", *set_find(m, "nice")));
    assert( 0 == strcmp("hash", *set_find(m, "hash")));

    // reinsertion
    assert(set_insert(m, "hello"));
    assert(0 == strcmp("hello", *set_find(m, "hello")));

    set_free(m);

    assert( !puts("Ok") );
}

void set_benchmark() {
    #ifndef NUM
    #define NUM 2000000
    #endif

    set(int) m;
    set_init(m, less_int, hash_int);

    clock_t t0 = clock();

    for( int i = 0; i < NUM; ++i ) {
        set_insert(m, i);
    }
    for( int i = 0; i < NUM; ++i ) {
        int *v = set_find(m, i);
        assert( v && *v == i );
    }

    double t = (clock() - t0) / (double)CLOCKS_PER_SEC;

    printf("[0]=%d\n", *set_find(m, 0));
    printf("[N-1]=%d\n", *set_find(m, NUM-1));
    printf("%d ops in %5.3fs = %fM ops/s\n", (NUM*2), t, (NUM*2) / (1e6 * t) );

    set_free(m);
}

int main() {
    set_tests();
    puts("---");
    set_tests2();
    puts("---");
    set_benchmark();
    assert(~puts("Ok"));
}

#define main main__
#endif // SET_DEMO
#endif // SET_C
