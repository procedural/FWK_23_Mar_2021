// game framework
// - rlyeh, public domain

#ifndef FWK_H
#define FWK_H

#include <math.h>
#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// -----------------------------------------------------------------------------
// new C keywords
#define countof(x)   (sizeof (x) / sizeof 0[x])
#ifdef _WIN32
#define threadlocal  __declspec(thread)
#elif __linux__
#define threadlocal  __thread
#endif

// -----------------------------------------------------------------------------
// directives (debug /O0 /D3 > debugopt /O1 /D2 > release (ndebug) /O2 /D1 > final /O3 /D0)

#ifdef FINAL
#define WITH_PROFILE       0
#define WITH_COOKER        0
#define WITH_FASTCALL_LUA  1
#define WITH_LEAK_DETECTOR 0
#endif

// defaults:

#ifndef WITH_PROFILE
#define WITH_PROFILE 1
#endif

#ifndef WITH_COOKER
#define WITH_COOKER 1
#endif

#ifndef WITH_FASTCALL_LUA
#define WITH_FASTCALL_LUA 0
#endif

#ifndef WITH_LEAK_DETECTOR
#define WITH_LEAK_DETECTOR 0
#endif

// -----------------------------------------------------------------------------
// forward includes

#define array(t) t*  // forward #include "3rd/3rd_ds.h"
#ifndef SYS_REALLOC
#define SYS_REALLOC realloc
#endif
#include "fwk_memory.h"
#include "fwk_math.h"
#include "fwk_collide.h"
//---
#include "fwk_audio.h"
#include "fwk_cooker.h"
#include "fwk_data.h"
#include "fwk_editor.h"
#include "fwk_file.h"
#include "fwk_input.h"
#include "fwk_math.h"
#include "fwk_network.h"
#include "fwk_render.h"
#include "fwk_renderdd.h"
#include "fwk_scene.h"
#include "fwk_script.h"
#include "fwk_system.h"
#include "fwk_ui.h"
#include "fwk_video.h"
#include "fwk_window.h"

#endif

// =============================================================================

#ifdef FWK_C
#pragma once

//-----------------------------------------------------------------------------
// new C macros
// @todo: AUTORUN macro (needed?), IFDEF_MSC macro (needed?)

#define ONCE                static int once##__LINE__ = 0; for(;!once##__LINE__;once##__LINE__=1) if(!once##__LINE__)
#define PRINTF(...)         PRINTF(stringf(__VA_ARGS__), 1[#__VA_ARGS__] == '!' ? callstack(+48) : "", __FILE__, __LINE__, __FUNCTION__)
#define PANIC(...)          PANIC(stringf(__VA_ARGS__), __FILE__, __LINE__)
#define WARNING(...)        (MessageBoxA(0,stringf(__VA_ARGS__),0,0), 0)
#define ASSERT(expr, ...)   do { int fool_msvc[] = {0,}; if(!(expr)) { fool_msvc[0]++; breakpoint(stringf("!Expression failed: " #expr " " FILELINE "\n" __VA_ARGS__)); } } while(0)
#define EXPAND(name, ...)   EXPAND_QUOTE(EXPAND_JOIN(name, EXPAND_COUNT_ARGS(__VA_ARGS__)), (__VA_ARGS__))
#define FILELINE            __FILE__ ":" STRINGIZE(__LINE__)

// implementation details

#define EXPAND_QUOTE(x, y)         x y
#define EXPAND_JOIN(name, count)   EXPAND_J0IN(name, count)
#define EXPAND_J0IN(name, count)   EXPAND_J01N(name, count)
#define EXPAND_J01N(name, count)   name##count
#define EXPAND_COUNT_ARGS(...)     EXPAND_ARGS((__VA_ARGS__, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0))
#define EXPAND_ARGS(args)          EXPAND_RETURN_COUNT args
#define EXPAND_RETURN_COUNT(_1_, _2_, _3_, _4_, _5_, _6_, _7_, _8_, _9_, count, ...) count

#define STRINGIZE(x)       STRINGIZ3(x)
#define STRINGIZ3(x)       #x

void (PRINTF)(const char *text, const char *stack, const char *file, int line, const char *function);

// ----------------------------------------------------------------------------
// 3rd party libs

#define ARCHIVE_C                           // archive.c
#define COMPRESS_C                          // compress.c
#define DS_C                                // ds.c
#define GJK_C                               // gjk
#define GLAD_GL_IMPLEMENTATION              // glad
#define _GLFW_WIN                           // glfw
#define GLFW_INCLUDE_NONE                   // glfw
#define LSH_GLFW_IMPLEMENTATION             // glfw
#define JAR_MOD_IMPLEMENTATION              // jar_mod
#define JAR_XM_IMPLEMENTATION               // jar_xm
#define JO_MPEG_COMPONENTS 3                // jo_mpeg
#define JSON5_C                             // json5
#define LUA_IMPL                            // lua542
#define MINIAUDIO_IMPLEMENTATION            // miniaudio
#define NK_GLFW_GL3_IMPLEMENTATION          // nuklear
#define NK_IMPLEMENTATION                   // nuklear
#define NK_INCLUDE_DEFAULT_ALLOCATOR        // nuklear
#define NK_INCLUDE_DEFAULT_FONT             // nuklear
#define NK_INCLUDE_FIXED_TYPES              // nuklear
#define NK_INCLUDE_FONT_BAKING              // nuklear
#define NK_INCLUDE_STANDARD_IO              // nuklear
#define NK_INCLUDE_STANDARD_VARARGS         // nuklear
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT     // nuklear
#define NK_KEYSTATE_BASED_INPUT             // nuklear
#define PL_MPEG_IMPLEMENTATION              // pl_mpeg
#define STB_IMAGE_IMPLEMENTATION            // stbi
#define STB_IMAGE_WRITE_IMPLEMENTATION      // stbi_write
#define STS_MIXER_IMPLEMENTATION            // sts_mixer
#define SWRAP_IMPLEMENTATION                // swrap
#define SWRAP_STATIC                        // swrap
#define THREAD_IMPLEMENTATION               // thread

#include "3rd/3rd_ds_safe.h" // 3rd_ds.h
//---
#include "3rd/3rd_glad.h"
#include "3rd/3rd_glfw3.h"
#undef timeGetTime
//---
#include "3rd/3rd_swrap.h"
//---
#include "3rd/3rd_jo_mp1.h"
#include "3rd/3rd_sfxr.h"
#define get_bits stb_vorbis_get_bits
#define error stb_vorbis_error
#include "3rd/3rd_stb_vorbis.h"
#undef error
#define channel jar_channel
#include "3rd/3rd_jar_mod.h"
#undef channel
#undef DEBUG
#include "3rd/3rd_jar_xm.h"
#include "3rd/3rd_sts_mixer.h"
#include "3rd/3rd_miniaudio.h"
//---
#undef L
#undef C
#undef R
#define error l_error
#include "3rd/3rd_lua542.h"
//---
#include "3rd/3rd_stb_image.h"
#include "3rd/3rd_stb_image_write.h"
//---
#undef freelist
#include "3rd/3rd_nuklear.h"
#include "3rd/3rd_nuklear_glfw_gl3.h"
//---
#ifdef WITH_ASSIMP
#include "3rd/3rd_assimp.h"
#endif
#include "3rd/3rd_json5.h"
#include "3rd/3rd_gjk.h"
#include "3rd/3rd_compress.h"
#include "3rd/3rd_archive.h"
#include "3rd/3rd_thread.h"
#include "3rd/3rd_plmpeg.h"
#include "3rd/3rd_jo_mpeg.h"

// ----------------------------------------------------------------------------
// compat

#ifdef _MSC_VER
#include <omp.h> // compile with /openmp to speed up some computations
#endif

#ifdef _MSC_VER
#include <stdio.h>
#include <windows.h>
#include <share.h>
#include <io.h>
#include <fcntl.h>
#include <sys/stat.h>
FILE *fmemopen(void *buf, size_t len, const char *type) {
    int fd = -1;
    char temppath[MAX_PATH - 14], filename[MAX_PATH + 1];
    if( GetTempPathA(sizeof(temppath), temppath) )
    if( GetTempFileNameA(temppath, "fwk_temp", 0, filename) )
    if( !_sopen_s(&fd, filename, _O_CREAT | _O_SHORT_LIVED | _O_TEMPORARY | _O_RDWR | _O_BINARY | _O_NOINHERIT, _SH_DENYRW, _S_IREAD | _S_IWRITE) )
    for( FILE *fp = fd != -1 ? _fdopen(fd, "w+b") : NULL; fp; )
    return fwrite(buf, len, 1, fp), rewind(fp), fp; // no need to _close. fclose(on the returned FILE*) also _closes the file descriptor.
    return fd != -1 ? _close(fd), NULL : NULL;
}
#endif

#ifdef _MSC_VER
// Find substring in string, case insensitive. Alias for strcasestr()
// Returns first char of coincidence, or NULL.
static const char *strstri( const char *str, const char *find ){
    while( *str++ ) {
        for( const char *s = str-1, *f = find, *c = s; ; ++f, ++c) {
            if(!*f) return s;
            if(!*c) return NULL;
            if(tolower(*c) != tolower(*f)) break;
        }
    }
    return NULL;
}

// Safely concatenate two strings. Always NUL terminates (unless dstcap == 0).
// Returns length of operation; if retval >= dstcap, truncation occurred.
static size_t strlcat(char *dst, const char *src, size_t dstcap) {
    int dl = strlen(dst), sl = strlen(src);
    if( dstcap ) snprintf(dst + dl, dstcap - dl, "%s", src);
    return dl + sl;
}
// Safely copy two strings. Always NUL terminates (unless dstcap == 0).
// Copy src to string dst of size dstcap. Copies at most dstcap-1 characters.
// Returns length of input; if retval >= dstcap, truncation occurred.
static size_t strlcpy(char *dst, const char *src, size_t dstcap) {
    int sl = strlen(src);
    if( dstcap ) snprintf(dst, dstcap, "%*s", sl, src);
    return sl;// count does not include NUL
}
#endif

static char *wchar16to8_(const wchar_t *str) { // from wchar16(win) to utf8/ascii
    int i = 0;
    int n = wcslen(str) * 6 - 1;
    static threadlocal char buffer[2048]; assert( n < 2048 );
    while( *str ) {
       if (*str < 0x80) {
          if (i+1 > n) return NULL;
          buffer[i++] = (char) *str++;
       } else if (*str < 0x800) {
          if (i+2 > n) return NULL;
          buffer[i++] = 0xc0 + (*str >> 6);
          buffer[i++] = 0x80 + (*str & 0x3f);
          str += 1;
       } else if (*str >= 0xd800 && *str < 0xdc00) {
          uint32_t c;
          if (i+4 > n) return NULL;
          c = ((str[0] - 0xd800) << 10) + ((str[1]) - 0xdc00) + 0x10000;
          buffer[i++] = 0xf0 + (c >> 18);
          buffer[i++] = 0x80 + ((c >> 12) & 0x3f);
          buffer[i++] = 0x80 + ((c >>  6) & 0x3f);
          buffer[i++] = 0x80 + ((c      ) & 0x3f);
          str += 2;
       } else if (*str >= 0xdc00 && *str < 0xe000) {
          return NULL;
       } else {
          if (i+3 > n) return NULL;
          buffer[i++] = 0xe0 + (*str >> 12);
          buffer[i++] = 0x80 + ((*str >> 6) & 0x3f);
          buffer[i++] = 0x80 + ((*str     ) & 0x3f);
          str += 1;
       }
    }
    buffer[i] = 0;
    return buffer;
}
static char *wchar16to8(const wchar_t *str) { // from wchar16(win) to utf8/ascii
    char *result = wchar16to8_(str);
    return stringf("%s", result ? result : "");
}

#if 0
static
const char *pathfile_from_handle(FILE *fp) {
#ifdef _WIN32
    int fd = fileno(fp);
    HANDLE handle = (HANDLE)_get_osfhandle( fd ); // <io.h>
    DWORD size = GetFinalPathNameByHandleW(handle, NULL, 0, VOLUME_NAME_DOS);
    wchar_t name[MAX_PATH] = L"";
    size = GetFinalPathNameByHandleW(handle, name, size, VOLUME_NAME_DOS);
    name[size] = L'\0';
    return wchar16to8(name + 4); // skip \\?\ header
#else
    // In Linux, you can use readlink on /proc/self/fd/NNN where NNN is the file descriptor
    // In OSX:
    //     #include <sys/syslimits.h>
    //     #include <fcntl.h>
    //     char filePath[PATH_MAX];
    //     if (fcntl(fd, F_GETPATH, filePath) != -1) {
    //         // do something with the file path
    //     }
    return 0;
#endif
}
#endif

// -----------------------------------------------------------------------------
// profiler & stats (@fixme: threadsafe)

#if WITH_PROFILE
#   define profile_init() map_init(profiler, less_str, hash_str)
#   define profile(...) for( \
        struct profile_t *found = map_find_or_add(profiler, #__VA_ARGS__ "@" FILELINE, (struct profile_t){NAN} ), *dummy = (\
        found->cost = -time_ms() * 1000, found); found->cost < 0; found->cost += time_ms() * 1000, found->avg = found->cost * 0.25 + found->avg * 0.75)
#   define profile_incstat(name, accum) do { if(profiler) { \
        struct profile_t *found = map_find(profiler, name); \
        if(!found) found = map_insert(profiler, name, (struct profile_t){0}); \
        found->stat += accum; \
        } } while(0)
#   define profile_render() if(profiler) do { \
        for(float _i = ui_begin("Profiler",0), _r; _i ; ui_end(), _i = 0) { \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( !isnan(val->stat) ) ui_slider2(stringf("Stat: %s", *key), (_r = val->stat, &_r), stringf("%.2f ", val->stat)), val->stat = 0; \
            ui_separator(); \
            for each_map_ptr(profiler, const char *, key, struct profile_t, val ) \
                if( isnan(val->stat) ) ui_slider2(*key, (_r = val->avg/1000.0, &_r), stringf("%.2f ms ", val->avg/1000.0)); \
        } } while(0)
struct profile_t { double stat; int32_t cost, avg; };
static map(char *, struct profile_t) profiler = 0;
#else
#   define profile_init() do {} while(0)
#   define profile_incstat(name, accum) do {} while(0)
#   define profile(...) if(1) // for(int _p = 1; _p; _p = 0)
#   define profile_render()
#endif

#define benchmark for(double t = -time_ss(); t < 0; PRINTF("%.2fs\n", t+=time_ss()))

// -----------------------------------------------------------------------------

#define AUDIO_C
#define COLLIDE_C
#define COOKER_C
#define DATA_C
#define EDITOR_C
#define FILE_C
#define INPUT_C
#define MATH_C
#define MEMORY_C
#define NETWORK_C
#define RENDER_C
#define RENDERDD_C
#define SCENE_C
#define SCRIPT_C
#define SYSTEM_C
#define UI_C
#define VIDEO_C
#define WINDOW_C

void fwk_init();
static void fwk_pre_init_systems();
static void fwk_post_init_systems();
static void fwk_pre_swap_systems();

#define  atexit(...) // hack to boost exit time. there are no critical systems that need to quit properly
#include "fwk_audio.h"
#include "fwk_collide.h"
#include "fwk_cooker.h"
#include "fwk_data.h"
#include "fwk_editor.h"
#include "fwk_file.h"
#include "fwk_input.h"
#include "fwk_math.h"
#include "fwk_memory.h"
#include "fwk_network.h"
#include "fwk_render.h"
#include "fwk_renderdd.h"
#include "fwk_scene.h"
#include "fwk_script.h"
#include "fwk_system.h"
#include "fwk_ui.h"
#include "fwk_video.h"
#include "fwk_window.h"

// ----------------------------------------------------------------------------
//static int threadlocal _thread_id;
//#define PRINTF(...)      (printf("%03d %07.3fs|%-16s|", (((unsigned)(uintptr_t)&_thread_id)>>8) % 1000, time_ss(), __FUNCTION__), printf(__VA_ARGS__), printf("%s", 1[#__VA_ARGS__] == '!' ? callstack(+48) : "")) // verbose logger

// ----------------------------------------------------------------------------

// OK: LZMA,DEFL,LZ4X,ULZ,BCM,CRSH,BALZ,LZW3,LZSS,LZP1,PPP,RAW
// NO: LZP1 @fixme

#ifndef COOKER_COMPRESSION
#define COOKER_COMPRESSION LZ4X|0 // Use COMPRESSOR|LEVEL[0..15]
#endif

#ifndef COOKER_CALLBACK
#define COOKER_CALLBACK fwk_cook
#endif

#define cookme(...) cookme(stringf(__VA_ARGS__))
static int (cookme)(const char *cmd) {
    // reserve batch file for forensic purposes
    ONCE unlink(".cook.bat");

    int rc = os_exec(cmd);

    if(0) // <-- uncomment to debug pipeline logs
    for( FILE* fp = fopen(".cook.bat", "a+b"); fp; fclose(fp), fp = 0) {
        fputs(cmd, fp);
        fputs("\r\n", fp);
        fprintf(fp, "REM Returned: %d [%#x]\r\n", rc, rc);
        fputs("\r\n", fp);
    }

    return rc;
}

static
int fwk_cook(char *filename, const char *ext, const char header[16], FILE *in, FILE *out, const char *info) {
    // reserve i/o buffer (2 MiB)
    enum { BUFSIZE = 2 * 1024 * 1024 };
    static char *buffer = 0; if(!buffer) buffer = REALLOC(0, BUFSIZE);

    // exclude extension-less files
    if( !ext[0] ) goto bypass;
    // exclude vc c/c++ .obj files
    if( !strcmp(ext, ".obj") && !memcmp(header, "\x64\x86", 2)) goto bypass;

    // exclude anything which is not supported
    ext = stringf("%s.", ext); // ".c" -> ".c."
    int is_supported = !!strstr(
        ".image.jpg.jpeg.png.tga.bmp.psd.hdr.pic.pnm"
        ".model.iqm.gltf.gltf2.fbx.obj.dae.blend.md3.md5.ms3d.smd.x.3ds.bvh.dxf.lwo"
        ".audio.wav.mod.xm.flac.ogg.mp1.mp3.mid"
        ".data.json.xml.csv.ini.cfg.doc.txt.md"
        ".shader.glsl.vs.fs"
        ".script.lua.tl"
        ".video.mpg.mpeg" ".", ext);
    if( !is_supported ) goto bypass;

    int must_process_model = !!strstr(".model.gltf.gltf2.fbx.obj.dae.blend.md3.md5.ms3d.smd.x.3ds.bvh.dxf.lwo" ".", ext); // note: no .iqm here
    int must_process_audio = !!strstr(".audio.mid" ".", ext);
    int must_process = must_process_model || must_process_audio;

    if( !must_process ) {
        // read -> write
        for( int n; !!(n = fread( buffer, 1, BUFSIZE, in )); ){
            bool ok = fwrite( buffer, 1, n, out ) == n;
            if( !ok ) goto failed;
        }
    } else {
        // read -> process -> write
        puts(info);
        float dt = -time_ss();

        tty_color(GREEN);

        if( must_process_model ) {
            const char *option_flip_uv = 0==strcmp(ext,".dae" ".") ? "-U" : "";

            int rc;
            const char *infile, *outfile;
            char temp_iqe[16] = ".temp.iqe";
            char temp_iqm[16] = ".temp.iqm";

            rc = cookme("3rd\\3rd_tools\\ass2iqe %s -o %s \"%s\"", option_flip_uv, outfile = temp_iqe, infile = filename);
            tty_color(rc || !file_size(outfile) ? RED : color_from_textlog(os_exec_output()));
            printf("%s\nReturned: %d (%#x)\n", os_exec_output(), rc, rc);

            rc = cookme("3rd\\3rd_tools\\iqe2iqm %s %s", outfile = temp_iqm, infile = temp_iqe);
            tty_color(rc || !file_size(outfile) ? RED : color_from_textlog(os_exec_output()));
            printf("%s\nReturned: %d (%#x)\n", os_exec_output(), rc, rc);

            if( !file_size(outfile) ) goto failed;
            for( FILE *in_ = fopen(outfile, "rb"); in_; fclose(in_), in_ = 0) {
                for( int n; !!(n = fread( buffer, 1, BUFSIZE, in_ )); ){
                    bool ok = fwrite( buffer, 1, n, out ) == n;
                    if( !ok ) { unlink(temp_iqe), unlink(temp_iqm); goto failed; }
                }
            }

            unlink(temp_iqe), unlink(temp_iqm);
        }
        if( must_process_audio ) {
            const char *option_soundbank_file = "3rd\\3rd_tools\\AweROMGM.sf2";

            int rc;
            const char *infile, *outfile;
            char temp_wav[16] = ".temp.wav";

            rc = cookme("3rd\\3rd_tools\\mid2wav \"%s\" %s %s", infile = filename, outfile = temp_wav, option_soundbank_file);
            tty_color(rc || !file_size(outfile) ? RED : color_from_textlog(os_exec_output()));
            printf("%s\nReturned: %d (%#x)\n", os_exec_output(), rc, rc);

            if( !file_size(outfile) ) goto failed;
            for( FILE *in_ = fopen(outfile, "rb"); in_; fclose(in_), in_ = 0) {
                for( int n; !!(n = fread( buffer, 1, BUFSIZE, in_ )); ){
                    bool ok = fwrite( buffer, 1, n, out ) == n;
                    if( !ok ) { unlink(temp_wav); goto failed; }
                }
            }

            unlink(temp_wav);
        }

        dt += time_ss(); printf("%.2fs\n\n", dt);
        tty_color(0);
    }

    // return compression level
    // exclude non-compressible files (jpg,mp3,...) -> lvl 0
    // exclude also files that compress a little bit, but we better leave them raw inside zip for streaming purposes (like wavs) -> lvl 0
    // exclude also infiles whose outfiles are one of the above (mid->wav)
    int level = COOKER_COMPRESSION;
    return strstr(".jpg.jpeg.png.flac.ogg.mp1.mp3.mpg.mpeg.wav.mid" ".", ext) ? 0 : level;

    bypass: return errno = 0, -1;
    failed: return errno = -1;
}

static void fwk_pre_init_systems() {
    profile_init();
    ddraw_init();
    sprite_init();

    // window_swap();

    script_init();
    audio_init(0);
}
static void fwk_post_init_systems() {
    // mount virtual filesystems
    // if(!vfs_mount("./"))        PANIC("cannot mount fs: ./");

    for( int i = 0; i < 16; ++i)
    if(!vfs_mount(stringf(".cook[%d].zip", i))) {}; // PANIC("cannot mount fs: .cook[%d].zip", i);

    // create an empty scene by default
    scene_push();

    input_init();

    errno = 0;
}
static void fwk_pre_swap_systems() {
    // flush all batched sprites before swap
    sprite_update();

    // flush all debugdraw calls before swap
    // ddraw_flush(); // @fixme: breaks either test_script or test(FXs)

    // flush all batched ui before swap (creates single dummy if no batches are found)
    ui_create();
    ui_render();
}

static
void fwk_error_callback(int error, const char *description) {
    PANIC("%s", description);
}

void fwk_init() {
    static int once = 0; if(once) return; once=1;

    // init panic handler
    (PANIC)(0,0,0);

    // enable ansi console
    tty_init();

    // init glfw
    glfwSetErrorCallback(fwk_error_callback);
    glfwInit();
    atexit(glfwTerminate);

    // create or update .cook file
#if WITH_COOKER
    cooker( "**", COOKER_CALLBACK, 0|COOKER_ASYNC );
#endif
}

#endif
