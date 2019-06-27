#pragma once
#ifdef _WIN32
#include <io.h>
#endif
#ifndef _WIN32
#include <sys/stat.h>

#ifdef __MACH__
#include <sys/disk.h>
#endif
#ifdef __linux__
#include <linux/fs.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>
#endif


// wrap posix file handle in a c++ class,
// so it will be closed when it leaves the current scope.
// This is most useful for making code exception safe.
struct filehandle {
    int f=-1;
    filehandle(int f) : f(f) { 
        if (f==-1) {
            perror("open");
            throw std::runtime_error("invalid filehandle");
        }
    }
    ~filehandle() { if (f!=-1) close(f); }
    filehandle& operator=(int fh) { f= fh; return *this; }
    operator int () const { return f; }

    int64_t size()
    {
        struct stat st;
        if (fstat(f, &st)) {
            print("ERROR: lstat");
            return -1;
        }
        if (st.st_mode&S_IFREG)
            return st.st_size;
        else if (st.st_mode&S_IFBLK) {
#ifdef DKIOCGETBLOCKCOUNT
            uint64_t bkcount;
            uint32_t bksize;
            if (-1==ioctl(f, DKIOCGETBLOCKCOUNT, &bkcount)) {
                print("ERROR: ioctl(DKIOCGETBLOCKCOUNT)");
                return -1;
            }
            if (-1==ioctl(f, DKIOCGETBLOCKSIZE, &bksize)) {
                print("ERROR: ioctl(DKIOCGETBLOCKSIZE)");
                return -1;
            }
            return bkcount*bksize;
#endif
#ifdef BLKGETSIZE64
            uint64_t devsize;
            if (-1==ioctl(f, BLKGETSIZE64, &devsize)) {
                print("ERROR: ioctl(BLKGETSIZE64)");
                return -1;
            }
            return devsize;
#endif
        }
        else {
            // pipe or socket
            printf("could not get size for device\n");
            return -1;
        }
    }
};
