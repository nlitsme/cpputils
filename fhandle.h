#pragma once
#include <stdexcept>
#include <system_error>

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

/*
 * NOTE: problem on macosx: you can't mmap a block or char device.
 *
 * see https://stackoverflow.com/questions/24520474/mmap-a-block-device-on-mac-os-x
 *
 * the problem is in the mmap kernel driver:
https://opensource.apple.com/source/xnu/xnu-2422.1.72/bsd/kern/kern_mman.c

        if (vp->v_type != VREG && vp->v_type != VCHR) {
			(void)vnode_put(vp);
			error = EINVAL;
			goto bad;
		}
        ...
		if (vp->v_type == VCHR || vp->v_type == VSTR) {
			(void)vnode_put(vp);
			error = ENODEV;
			goto bad;
		} 

 */
// wrap posix file handle in a c++ class,
// so it will be closed when it leaves the current scope.
// This is most useful for making code exception safe.

// NOTE: this object should not be copied.

struct filehandle {
    int f=-1;
    filehandle(int f) : f(f) { 
        if (f==-1) {
            throw std::system_error(errno, std::generic_category());
        }
    }

    // disallow copying - there may only be one owner.
    filehandle(const filehandle & fh) = delete;

    ~filehandle() { if (f!=-1) close(f); }
    filehandle& operator=(int fh) { f= fh; return *this; }
    operator int () const { return f; }

    int64_t size()
    {
        struct stat st;
        if (fstat(f, &st))
            return -1;
        if (st.st_mode&S_IFREG)
            return st.st_size;
        else if (st.st_mode&S_IFBLK) {
#ifdef DKIOCGETBLOCKCOUNT
            uint64_t bkcount;
            uint32_t bksize;
            if (-1==ioctl(f, DKIOCGETBLOCKCOUNT, &bkcount))
                return -1;
            if (-1==ioctl(f, DKIOCGETBLOCKSIZE, &bksize))
                return -1;
            return bkcount*bksize;
#endif
#ifdef BLKGETSIZE64
            uint64_t devsize;
            if (-1==ioctl(f, BLKGETSIZE64, &devsize))
                return -1;
            return devsize;
#endif
        }
        else {
            // pipe or socket
            return -1;
        }
    }
};
