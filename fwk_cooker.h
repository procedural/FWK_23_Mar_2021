// asset pipeline framework
// - rlyeh, public domain.
//
// all cooked assets are stored inside .cook.zip file at root folder, which acts as an asset database.
// during game boot, the database gets rebuilt as follows:
// 1. compare local disk files against file in zip database. for each mismatch do:
// 2. - invalidate its entry in database, if local file was removed from disk.
// 3. - write its *cooked* contents into database, if local file was created or modified from disk.
//
// notes: meta-datas from every raw asset are stored into comment field, inside .cook.zip archive.
// @todo: fix leaks
// @todo: symlink exact files
// @todo: parallelize list of files in N cores. get N .cook files instead. mount them all.

#ifndef COOKER_H
#define COOKER_H

enum {
    COOKER_ASYNC = 1,
};

// user defined callback for asset cooking:
// must read infile, process data, and write it to outfile
// must set errno on exit if errors are found
// must return compression level if archive needs to be cooked, else return <0
typedef int (*cooker_callback_t)(char *filename, const char *ext, const char header[16], FILE *in, FILE *out, const char *info);

int  cooker_progress(); // [0..100]
bool cooker( const char *masks, cooker_callback_t cb, int flags );

#endif

// -----------------------------------------------------------------------------

#ifdef COOKER_C
#pragma once

#ifdef _MSC_VER
#include <io.h>
#else
#include <unistd.h>
#endif
#ifndef PATH_MAX
#define PATH_MAX MAX_PATH
#endif

#ifndef COOKER_TMPFILE
#define COOKER_TMPFILE ".temp" // tmpnam(0) // ".temp"
#endif

typedef struct fs {
    char *fname, status;
    uint64_t stamp;
    uint64_t bytes;
} fs;

struct cooker_args {
    const char **files;
    cooker_callback_t callback;
    char zipfile[16];
    int from, to;
};

static
array(fs) cooker__fs_scan(struct cooker_args *args) {
    array(struct fs) fs = 0;

    // iterate all previously scanned files
    for( int i = args->from; i < args->to; ++i ) {
        const char *fname = args->files[i];
        if( file_directory(fname) ) continue; // skip folders

        // @todo: normalize path & rebase here (absolute to local)
        // [...]
        // fi.normalized = ; tolower->to_underscore([]();:+ )->remove_extra_underscores

        // make buffer writable
        char buffer[PATH_MAX];
        snprintf(buffer, PATH_MAX, "%s", fname);

        // get normalized current working directory (absolute)
        char cwd[PATH_MAX] = {0};
        getcwd(cwd, sizeof(cwd));
        for(int i = 0; cwd[i]; ++i) if(cwd[i] == '\\') cwd[i] = '/';

        // normalize path
        for(int i = 0; buffer[i]; ++i) if(buffer[i] == '\\') buffer[i] = '/';

        // rebase from absolute to relative
        char *buf = buffer; int cwdlen = strlen(cwd);
        if( !strncmp(buf, cwd, cwdlen) ) buf += cwdlen;
        while(buf[0] == '/') ++buf;

        if( file_name(buf)[0] == '.' ) continue; // skip system files

        struct fs fi = {0};
        fi.fname = STRDUP(buf);
        fi.bytes = file_size(buf);
        fi.stamp = file_stamp_human(buf); // human-readable base10 timestamp

        array_push(fs, fi);
    }
    return fs;
}

static
fs *cooker__fs_locate(array(struct fs) fs, const char *file) {
    for(int i = 0; i < array_count(fs); ++i) {
        if( !strcmp(fs[i].fname,file)) {
            return &fs[i];
        }
    }
    return 0;
}

static array(char*) added;
static array(char*) changed;
static array(char*) deleted;
static array(char*) uncooked;

static
int cooker__fs_diff( zip* old, array(fs) now ) {
    array_free(added);
    array_free(changed);
    array_free(deleted);
    array_free(uncooked);

    // if not zipfile is present, all files are new and must be added
    if( !old ) {
        for( int i = 0; i < array_count(now); ++i ) {
            array_push(uncooked, STRDUP(now[i].fname));
        }
        return 1;
    }

    // compare for new & changed files
    for( int i = 0; i < array_count(now); ++i ) {
        int found = zip_find(old, now[i].fname);
        if( found < 0 ) {
            array_push(added, STRDUP(now[i].fname));
            array_push(uncooked, STRDUP(now[i].fname));
        } else {
            uint64_t oldsize = _atoi64(zip_comment(old,found)); // zip_size(old, found); returns sizeof processed asset. return original size of unprocessed asset, which we store in comment section
            uint64_t oldstamp = _atoi64(zip_modt(old, found)+20);
            if( oldsize != now[i].bytes || abs(oldstamp - now[i].stamp) > 1 ) { // @fixme: should use hash instead. hashof(tool) ^ hashof(args used) ^ hashof(rawsize) ^ hashof(rawdate)
                printf("%s:\t%llu vs %llu, %llu vs %llu\n", now[i].fname, (uint64_t)oldsize,(uint64_t)now[i].bytes, (uint64_t)oldstamp,(uint64_t)now[i].stamp);
                array_push(changed, STRDUP(now[i].fname));
                array_push(uncooked, STRDUP(now[i].fname));
            }
        }
    }
    // compare for deleted files
    for( int i = 0; i < zip_count(old); ++i ) {
        char *oldname = zip_name(old, i);
        int idx = zip_find(old, oldname); // find latest versioned file in zip
        unsigned oldsize = zip_size(old, idx);
        if (!oldsize) continue;
        fs *found = cooker__fs_locate(now, oldname);
        if( !found ) {
            array_push(deleted, STRDUP(oldname));
        }
    }
    return 1;
}

static volatile int cooker__progress = 0;

int cooker_progress() {
    return cooker__progress;
}

static
int cooker_sync( void *userptr ) {
    struct cooker_args *args = userptr;
    ASSERT( args && args->callback );

    array(struct fs) now = cooker__fs_scan(args);
    //printf("Scanned: %d items found\n", array_count(now));

    if( file_size(args->zipfile) == 0 ) unlink(args->zipfile);

    // populate added/deleted/changed arrays by examining current disk vs last cache
    zip *z = zip_open(args->zipfile, "r+b");
    cooker__fs_diff(z, now);
    if( z ) zip_close(z);

    fflush(0);

    z = zip_open(args->zipfile, "a+b");
    if( !z ) {
        unlink(args->zipfile);
        z = zip_open(args->zipfile, "a+b"); // try again
        if(!z) PANIC("cannot open file for updating: %s", args->zipfile);
    }

    // deleted files
    for( int i = 0, end = array_count(deleted); i < end; ++i ) {
        printf("Deleting %03d%% %s\n", (i+1) == end ? 100 : (i * 100) / end, deleted[i]);
        FILE* out = fopen(COOKER_TMPFILE, "wb"); fclose(out);
        FILE* in = fopen(COOKER_TMPFILE, "rb");
        char *comment = "0";
        zip_append_file(z, deleted[i], comment, in, 0);
        fclose(in);
    }
    // added or changed files
    // #pragma omp parallel for
    for( int i = 0, end = array_count(uncooked); i < end; ++i ) {
        cooker__progress = (i+1) == end ? 100 : (i * 100) / end; // (i+i>0) * 100.f / end;

        char *fname = uncooked[i];

        FILE *in = fopen(fname, "rb");
        if( !in ) PANIC("cannot open file for reading: %s", fname);
        fseek(in, 0L, SEEK_END);
        size_t inlen = ftell(in);
        fseek(in, 0L, SEEK_SET);

        unlink(COOKER_TMPFILE);
        FILE *out = fopen(COOKER_TMPFILE, "a+b");
        if( !out ) PANIC("cannot open .temp file for writing");
        fseek(out, 0L, SEEK_SET);

        char *ext = strrchr(fname, '.'); ext = ext ? ext : ""; // .jpg
        char header[16]; fread(header, 1, 16, in); fseek(in, 0L, SEEK_SET);

        const char *info = stringf("Cooking %03d%% %s\n", cooker__progress, uncooked[i]);
        int compression = (errno = 0, args->callback(fname, ext, header, in, out, info));
        int failed = errno != 0;
        if( failed ) PRINTF("importing failed: %s", fname);
        else if( compression >= 0 ) {
            fseek(out, 0L, SEEK_SET);
            char *comment = stringf("%d",(int)inlen);
            if( !zip_append_file(z, fname, comment, out, compression) ) {
                PANIC("failed to add processed file into %s: %s", args->zipfile, fname);
            }
        }

        fclose(in);
        fclose(out);
    }
    zip_close(z);

    unlink(COOKER_TMPFILE);
    fflush(0);

    cooker__progress = 100;
    return 1;
}

static
int cooker_async( void *userptr ) {
    while(!window_handle()) sleep_ms(100); // wait for window handle to be created

    int ret = cooker_sync(userptr);
    thread_exit( ret );
    return ret;
}

bool cooker( const char *masks, cooker_callback_t callback, int flags ) {
    static struct cooker_args args[1] = {0};
    const char **files = file_list(masks);
    int numfiles = 0; while(files[numfiles]) ++numfiles;
    args[0].files = files;
    args[0].callback = callback;
    args[0].from = 0;
    args[0].to = numfiles;
    for( int i = 0; i < countof(args); ++i) snprintf(args[i].zipfile, 16, ".cook[%d].zip", i);
    //
    if( flags & COOKER_ASYNC ) {
        int numthreads = countof(args);
        for( int i = 0; i < numthreads; ++i ) {
            args[i] = args[0];
            args[i].from = i == 0 ? 0 : args[i-1].to;
            args[i].to = i == (numthreads-1) ? numfiles : (numfiles * (i+1.) / numthreads);
            thread_ptr_t thd = thread_create( cooker_async, &args[i], "cooker_async()", 0/*STACK_SIZE*/ );
        }
        return true;
    } else {
        return !!cooker_sync( &args[0] );
    }
}

#endif
