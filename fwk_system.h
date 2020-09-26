// system framework utils
// - rlyeh, public domain.
//
// Note: Windows users add `/Zi` compilation flags, else add `-g` and/or `-ldl` flags
// Note: If you are linking your binary using GNU ld you need to add --export-dynamic

#ifndef SYSTEM_H
#define SYSTEM_H

char*       app_path();
void        app_reload();

int         os_argc();
char*       os_argv(int);

char*       os_exec_output();
int         os_exec(const char *command);
#define     os_exec(...) os_exec(stringf(__VA_ARGS__))

const char* os_option(const char *commalist, const char *defaults);
int         os_optioni(const char *commalist, int defaults);
float       os_optionf(const char *commalist, float defaults);

double      time_ss();
double      time_ms();
uint64_t    time_human(); // YYYYMMDDhhmmss
double      sleep_ss(double ss);
double      sleep_ms(double ms);

char*       callstack( int traces ); // write callstack into a temporary string. do not delete it.
int         callstackf( FILE *fp, int traces ); // write callstack to file. <0 traces to invert order.

uint16_t    lil16(uint16_t n); // swap16 as lil
uint32_t    lil32(uint32_t n); // swap32 as lil
float       lil32f(float n);   // swap32 as lil
uint64_t    lil64(uint64_t n); // swap64 as lil
double      lil64f(double n);  // swap64 as lil
uint16_t    big16(uint16_t n); // swap16 as big
uint32_t    big32(uint32_t n); // swap32 as big
float       big32f(float n);   // swap32 as big
uint64_t    big64(uint64_t n); // swap64 as big
double      big64f(double n);  // swap64 as big

uint16_t*   lil16p(void *n, int sz);
uint32_t*   lil32p(void *n, int sz);
float*      lil32pf(void *n, int sz);
uint64_t*   lil64p(void *n, int sz);
double*     lil64pf(void *n, int sz);
uint16_t*   big16p(void *n, int sz);
uint32_t*   big32p(void *n, int sz);
float*      big32pf(void *n, int sz);
uint64_t*   big64p(void *n, int sz);
double*     big64pf(void *n, int sz);

void        hexdump( FILE *fp, const void *ptr, unsigned len, int width );
void        breakpoint(const char *info);
bool        has_debugger();

void        tty_color(unsigned color);
void        tty_reset();

int (PANIC)(const char *error, const char *file, int line);

#endif

// -----------------------------------------------------------------------------

#ifdef SYSTEM_C
#pragma once

char *app_path() { // should return absolute path always
    static char buffer[1024] = {0};
    if( buffer[0] ) return buffer;
    unsigned length = GetModuleFileNameA(NULL, buffer, sizeof(buffer)); // @todo: use GetModuleFileNameW+wchar_t && convert to utf8 instead
    char *a = strrchr(buffer, '/');  if(!a) a = buffer + strlen(buffer);
    char *b = strrchr(buffer, '\\'); if(!b) b = buffer + strlen(buffer);
    char slash = (a < b ? *a : b < a ? *b : '/');
    return snprintf(buffer, 1024, "%.*s%c", length - (int)(a < b ? b - a : a - b), buffer, slash), buffer;
}

void app_reload() {
    // save_on_exit();
    fflush(0);
    chdir(app_path());
    execv(__argv[0], __argv);
    exit(0);
}

char* os_exec_output() {
    static threadlocal char os_exec__output[4096] = {0};
    return os_exec__output;
}
int (os_exec)( const char *cmd ) {
    int rc = -1;
    char *buf = os_exec_output(); buf[0] = 0; // memset(buf, 0, 4096);
    puts(cmd);
    for( FILE *fp = _popen( cmd, "rt" ); fp; rc = _pclose(fp), fp = 0) {
        while( fgets(buf, 4096 - 1, fp) ) {
            char *r = strrchr(buf, '\r'); if(r) *r = 0;
            char *n = strrchr(buf, '\n'); if(n) *n = 0;
        }
    }
    return rc;
}

#if defined(__APPLE__)
#include <execinfo.h> // backtrace, backtrace_symbols
#include <dlfcn.h>    // dladdr, Dl_info
#elif defined(__GNUC__)
#include <execinfo.h>  // backtrace, backtrace_symbols
#elif defined(_WIN32) // && !defined __TINYC__
#include <winsock2.h>  // windows.h alternative
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
#pragma comment(lib, "Kernel32")
static int backtrace( void **addr, int maxtraces ) { //$
    static HANDLE process = 0;
    if( !process ) process = GetCurrentProcess();
    if( !process ) exit( puts( "error: no current process" ) );
    static int init = 0;
    if( !init ) init = SymSetOptions(SYMOPT_UNDNAME), SymInitialize( process, NULL, TRUE );
    if( !init ) exit( puts( "error: cannot initialize DbgHelp.lib" ) );

    typedef USHORT (WINAPI *pFN)(); // TINYC
    //typedef USHORT (WINAPI *pFN)(__in ULONG, __in ULONG, __out PVOID*, __out_opt PULONG); // _MSC_VER
    static pFN rtlCaptureStackBackTrace = 0;
    if( !rtlCaptureStackBackTrace ) {
        rtlCaptureStackBackTrace = (pFN)GetProcAddress(LoadLibraryA("kernel32.dll"), "RtlCaptureStackBackTrace");
    }
    if( !rtlCaptureStackBackTrace ) {
        return 0;
    }
    return rtlCaptureStackBackTrace(1, maxtraces, (PVOID *)addr, (DWORD *) 0);
}
static char **backtrace_symbols(void *const *array,int size) { //$
    HANDLE process = GetCurrentProcess();
    enum { MAXSYMBOLNAME = 512 - sizeof(IMAGEHLP_SYMBOL64) };
    char symbol64_buf     [ 512 ];
    char symbol64_bufblank[ 512 ] = {0};
    IMAGEHLP_SYMBOL64 *symbol64       = (IMAGEHLP_SYMBOL64*)symbol64_buf;
    IMAGEHLP_SYMBOL64 *symbol64_blank = (IMAGEHLP_SYMBOL64*)symbol64_bufblank;
    symbol64_blank->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
    symbol64_blank->MaxNameLength = (MAXSYMBOLNAME-1) / 2; //wchar?

    int symlen = size * sizeof(char *);
    char **symbols = (char **)SYS_REALLOC(0, symlen);

    if( symbols ) {
        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = NULL;
        }

        char begin[1024];
        for( int i = 0; i < size; ++i ) {
            char **symbuf, *buffer = begin;

            DWORD64 dw1 = 0, dw2 = 0;
            *symbol64 = *symbol64_blank;
            if( SymGetSymFromAddr64( process, (DWORD64)array[i], &dw1, symbol64 ) ) {
                IMAGEHLP_LINE64 l64 = {0};
                l64.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
                if( SymGetLineFromAddr64( process, (DWORD64)array[i], (DWORD*)&dw2, &l64 ) ) {
                    // int lenbase( const char *str );
                    int base = 0; // lenbase( l64.FileName );
                    buffer += sprintf(buffer,"%s (%s:%d)%c", symbol64->Name, &l64.FileName[ base ], l64.LineNumber, 0);
                } else {
                    buffer += sprintf(buffer,"%s (??)%c", symbol64->Name, 0);
                }
            } else  buffer += sprintf(buffer,"(??)%c", 0);

            size_t buflen = buffer - begin + 1;

            symbuf = (char **)SYS_REALLOC( symbols, symlen + buflen );
            if( symbuf ) {
                memcpy( (char *)symbuf + symlen, begin, buflen );
                symbuf[ i ] = (char *)(size_t)symlen;

                symbols = symbuf;
                symlen += buflen;
            } else break;
        }

        for( int i = 0; i < size; ++i ) {
            symbols[ i ] = (char *)symbols + (size_t)symbols[ i ];
        }
    }

    return symbols;
}
#else
static int backtrace(void **heap, int num) { return 0; }
static char **backtrace_symbols(void *const *sym,int num) { return 0; }
#endif

void trace_cb( int traces, int (*yield)(const char *)) { //$
    enum { skip = 1 }; /* exclude 1 trace from stack (this function) */
    enum { maxtraces = 128 };

    int inc = 1;
    if( traces < 0 ) traces = -traces, inc = -1;
    if( traces == 0 ) return;
    if( traces > maxtraces ) traces = maxtraces;

    void *stack[ maxtraces ];
    traces = backtrace( stack, traces );
    char **symbols = backtrace_symbols( stack, traces );

    char demangled[1024], buf[1024];
    int L = 0, B = inc>0 ? skip - 1 : traces, E = inc>0 ? traces : skip - 1;
    for( int i = B; ( i += inc ) != E; ) {
#ifdef __linux__
        char *address = strstr( symbols[i], "[" ) + 1; address[strlen(address) - 1] = '\0';
        char *binary = symbols[i]; strstr( symbols[i], "(" )[0] = '\0';
        char command[1024]; sprintf(command, "addr2line -e %s %s", binary, address);
        FILE *fp = popen( command, "rb" );
        if( !fp ) {
            exit( puts( "cannot invoke 'addr2line'" ) );
        }
        char *line_p = fgets(demangled, sizeof(demangled), fp);
        symbols[i] = demangled;
        pclose(fp);
#elif __APPLE__
        struct Dl_info info;
        if( dladdr(stack[i], &info) && info.dli_sname ) {
            char *dmgbuf = info.dli_sname[0] != '_' ? NULL :
                 __cxa_demangle(info.dli_sname, NULL, 0, NULL);
            strcpy( demangled, dmgbuf ? dmgbuf : info.dli_sname );
            symbols[i] = demangled;
            FREE( dmgbuf );
        }
#endif
        sprintf(buf, "%03d: %#016p %s", ++L, stack[i], symbols[i]);
        //sprintf(buf, "%03d: %s", ++L, symbols[i]);
        if( yield(buf) < 0 ) break;
    }

    SYS_REALLOC( symbols, 0 );
}

static threadlocal char *trace_strbuf[128] = {0};
static threadlocal int trace_counter = 0, trace_len = 0;
int trace_(const char *line) {
    int len = strlen(line);
    trace_len += len + 1;
    trace_strbuf[trace_counter] = (char*)SYS_REALLOC(trace_strbuf[trace_counter], (len * 1.5));
    strcpy(trace_strbuf[trace_counter], line );
    trace_counter = (trace_counter +1) % 128;
    return 1;
}
char *callstack( int traces ) { //$
    //if( trace_ ) trace_str_ = SYS_REALLOC(trace_str_, trace_lenbuf_ = 0);
    trace_counter = trace_len = 0;
    trace_cb( traces, trace_ );
    static threadlocal char *buf = 0; // @fixme: 1 leak per invoking thread
    SYS_REALLOC(buf, 0);
    buf = (char*)SYS_REALLOC( 0, trace_len + 1 ); buf[0] = 0;
    for( int i = 0; i < trace_counter; ++i ) {
        strcat(buf, trace_strbuf[i] ); // <-- optimize
        strcat(buf, "\n");
    }
    return buf ? buf : ""; // @fixme: should return NULL if no callstack is retrieved?
}
int callstackf( FILE *fp, int traces ) { //$
    char *buf = callstack(traces);
    fputs(buf, fp);
    return 0;
}


#if defined(_MSC_VER)
#include <stdlib.h>
#define swap16 _byteswap_ushort
#define swap32 _byteswap_ulong
#define swap64 _byteswap_uint64
#elif defined __GNUC__
#define swap16 __builtin_bswap16
#define swap32 __builtin_bswap32
#define swap64 __builtin_bswap64
#else
uint16_t swap16( uint16_t x ) { return (x << 8) | (x >> 8); }
uint32_t swap32( uint32_t x ) { x = ((x << 8) & 0xff00ff00) | ((x >> 8) & 0x00ff00ff); return (x << 16) | (x >> 16); }
uint64_t swap64( uint64_t x ) { x = ((x <<  8) & 0xff00ff00ff00ff00ULL) | ((x >>  8) & 0x00ff00ff00ff00ffULL); x = ((x << 16) & 0xffff0000ffff0000ULL) | ((x >> 16) & 0x0000ffff0000ffffULL); return (x << 32) | (x >> 32); }
#endif

float    swap32f(float n)  { union { float  t; uint32_t i; } conv; conv.t = n; conv.i = swap32(conv.i); return conv.t; }
double   swap64f(double n) { union { double t; uint64_t i; } conv; conv.t = n; conv.i = swap64(conv.i); return conv.t; }

#define is_big()    ((*(uint16_t *)"\0\1") == 1)
#define is_little() ((*(uint16_t *)"\0\1") != 1)

uint16_t  lil16(uint16_t n) { return is_big()    ? swap16(n) : n; }
uint32_t  lil32(uint32_t n) { return is_big()    ? swap32(n) : n; }
uint64_t  lil64(uint64_t n) { return is_big()    ? swap64(n) : n; }
uint16_t  big16(uint16_t n) { return is_little() ? swap16(n) : n; }
uint32_t  big32(uint32_t n) { return is_little() ? swap32(n) : n; }
uint64_t  big64(uint64_t n) { return is_little() ? swap64(n) : n; }

float     lil32f(float n)  { return is_big()    ? swap32f(n) : n; }
double    lil64f(double n) { return is_big()    ? swap64f(n) : n; }
float     big32f(float n)  { return is_little() ? swap32f(n) : n; }
double    big64f(double n) { return is_little() ? swap64f(n) : n; }

uint16_t* lil16p(void *p, int sz)  { if(is_big()   ) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint16_t* big16p(void *p, int sz)  { if(is_little()) { uint16_t *n = (uint16_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap16(n[i]); } return p; }
uint32_t* lil32p(void *p, int sz)  { if(is_big()   ) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint32_t* big32p(void *p, int sz)  { if(is_little()) { uint32_t *n = (uint32_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap32(n[i]); } return p; }
uint64_t* lil64p(void *p, int sz)  { if(is_big()   ) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }
uint64_t* big64p(void *p, int sz)  { if(is_little()) { uint64_t *n = (uint64_t *)p; for(int i = 0; i < sz; ++i) n[i] = swap64(n[i]); } return p; }

float   * lil32pf(void *p, int sz) { if(is_big()   ) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
float   * big32pf(void *p, int sz) { if(is_little()) { float    *n = (float    *)p; for(int i = 0; i < sz; ++i) n[i] = swap32f(n[i]); } return p; }
double  * lil64pf(void *p, int sz) { if(is_big()   ) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }
double  * big64pf(void *p, int sz) { if(is_little()) { double   *n = (double   *)p; for(int i = 0; i < sz; ++i) n[i] = swap64f(n[i]); } return p; }


// ----------------------------------------------------------------------------
// time

double time_ss() {
    return glfwGetTime();
}
double time_ms() {
    return glfwGetTime() * 1000.0;
}
uint64_t time_human() {
    time_t mtime = time(0);
    struct tm *ti = localtime(&mtime);
    return _atoi64(stringf("%04d%02d%02d%02d%02d%02d",ti->tm_year+1900,ti->tm_mon+1,ti->tm_mday,ti->tm_hour,ti->tm_min,ti->tm_sec));
}
#if 0
uint64_t time_gpu() {
    GLint64 t = 123456789;
    glGetInteger64v(GL_TIMESTAMP, &t);
    return (uint64_t)t;
}
#endif
double sleep_ms(double ms) {
    double now = time_ms();
    if( ms <= 0 ) {
        Sleep(0); // yield
    } else {
        Sleep(ms);
    }
    return time_ms() - now;
}
double sleep_ss(double ss) {
    return sleep_ms( ss * 1000 ) / 1000.0;
}


// ----------------------------------------------------------------------------
// argc/v

#ifdef __GNUC__ // also, clang
    int __argc; char **__argv;
    __attribute__((constructor)) void init_argcv(int argc, char **argv) { __argc = argc; __argv = argv; }
#endif
int os_argc() { return __argc; }
char* os_argv(int arg) { return __argv[arg]; }

// ----------------------------------------------------------------------------
// options

const char *os_option(const char *commalist, const char *defaults) {
    while( commalist[0] ) {
        const char *begin = commalist;
        while(*commalist != ',' && *commalist != '\0') ++commalist;
        const char *end = commalist;

        char token[128];
        snprintf(token, 128, "%.*s", (int)(end - begin), begin);

        for( int i = 1; i < os_argc(); ++i ) {
            char *arg = os_argv(i);

            if( !strcmpi( arg, token ) ) {
                if( (i+1) < os_argc() ) {
                    return os_argv(i+1);
                }
            }
        }

        commalist = end + 1;
    }
    return defaults;
}

int os_optioni(const char *commalist, int defaults) {
    const char *rc = os_option(commalist, 0);
    return rc ? atoi(rc) : defaults;
}
float os_optionf(const char *commalist, float defaults) {
    const char *rc = os_option(commalist, 0);
    return rc ? atof(rc) : defaults;
}

// ----------------------------------------------------------------------------
// tty

void tty_color(unsigned color) {
    #ifdef _WIN32
    ONCE {
        unsigned mode = 0; SetConsoleMode(GetStdHandle(-11), (GetConsoleMode(GetStdHandle(-11), &mode), mode|4));
    }
    #endif
    if( color ) {
        // if( color == RED ) breakpoint("break on RED"); // debug
        unsigned r = (color >> 16) & 255;
        unsigned g = (color >>  8) & 255;
        unsigned b = (color >>  0) & 255;
        // 24-bit console ESC[ … 38;2;<r>;<g>;<b> … m Select RGB foreground color
        // 256-color console ESC[38;5;<fgcode>m
        // 0x00-0x07:  standard colors (as in ESC [ 30..37 m)
        // 0x08-0x0F:  high intensity colors (as in ESC [ 90..97 m)
        // 0x10-0xE7:  6*6*6=216 colors: 16 + 36*r + 6*g + b (0≤r,g,b≤5)
        // 0xE8-0xFF:  grayscale from black to white in 24 steps
        r /= 51, g /= 51, b /= 51; // [0..5]
        printf("\033[38;5;%dm", r*36+g*6+b+16); // "\033[0;3%sm", color_code);
    } else {
        printf("%s", "\x1B[39;49m"); // reset
    }
}
void tty_puts(unsigned color, const char *text) {
    tty_color(color); puts(text);
}
void tty_init() {
    tty_color(0);
}
int tty_cols() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO c;
    if( GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &c) ) {
        int w = c.srWindow.Right-c.srWindow.Left-c.dwCursorPosition.X;
        return w;
    }
#endif
    return 80;
}

// -----------------------------------------------------------------------------
// debugger

#include <stdio.h>
void hexdump( FILE *fp, const void *ptr, unsigned len, int width ) {
    unsigned char *data = (unsigned char*)ptr;
    for( unsigned jt = 0; jt < len; jt += width ) {
        fprintf( fp, "; %05d ", jt );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, "%02x %s", (unsigned char)data[it], &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
        fprintf( fp, "; %05d ", jt );
        for( unsigned it = jt, next = it + width; it < len && it < next; ++it ) {
            fprintf( fp, " %c %s", (signed char)data[it] >= 32 ? (signed char)data[it] : (signed char)'.', &" \n\0...\n"[ (1+it) < len ? 2 * !!((1+it) % width) : 3 ] );
        }
    }
}
void breakpoint(const char *line) {
    char *fulltext = line[0] == '!' ? stringf("%s\n%s", line+1, callstack(+48)) : line;
    window_visible(false);
    WARNING(fulltext);
#ifdef _MSC_VER
    do { \
        __try { DebugBreak(); } \
        __except (GetExceptionCode() == EXCEPTION_BREAKPOINT ? \
            EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH) {} \
    } while(0);
#endif
    window_visible(true);
}
bool has_debugger() {
#ifdef _WIN32
    return IsDebuggerPresent(); // SetLastError(123); OutputDebugStringA("\1"); enabled = GetLastError() != 123;
#else
    return false;
#endif
}

// ----------------------------------------------------------------------------
// logger

unsigned color_from_textlog(const char *text) {
    /**/ if( strstri(text, "fail") || strstri(text, "error") ) return RED;
    else if( strstri(text, "warn") || strstri(text, "not found") ) return YELLOW;
    return 0;
}

void (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function) {
    static thread_mutex_t lock, *init = 0; if(!init) thread_mutex_init(init = &lock);
    thread_mutex_lock( &lock );

        tty_color(/*errno ? RED :*/ color_from_textlog(text));
        #ifdef _MSC_VER
        char *slash = strrchr(file, '\\'); if(slash) file = slash + 1;
        #endif
        char *location = stringf("|%s|%s:%d", /*errno?strerror(errno):*/function, file, line);
        // errno = 0;
        printf("\r%*.s%s", tty_cols() + 1 - (int)strlen(location), "", location);
        printf("\r%07.3fs|%s%s", time_ss(), text, stack);
        tty_color(0);

    thread_mutex_unlock( &lock );
}

// ----------------------------------------------------------------------------
// panic

int (PANIC)(const char *error, const char *file, int line) {
    static char *reserve = 0; // for out-of-memory recovery
    reserve = SYS_REALLOC(reserve, reserve ? 0 : 4*1024*1024);
    if( line ) {
        tty_color(RED);

        error += error[0] == '!';
        fprintf(stderr, "Error: %s (%s:%d) (errno:%s)\n", error, file, line, strerror(errno));
        fprintf(stderr, "%s", callstack(+16)); // no \n
        fflush(0); // fflush(stderr);

        tty_color(0);

        breakpoint(error);
        exit(-line);
    }
    return 0;
}

#endif
