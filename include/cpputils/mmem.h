#pragma once
// class wrappong a memmapped object

#ifndef _WIN32
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#endif
#ifdef _WIN32
#include <windows.h>
#include <memoryapi.h>
#include <io.h>    // get_osfhandle
enum {
    PROT_READ = 1,
    PROT_WRITE = 2,
};

#endif

#include <stdint.h>
#include <algorithm>
#include <system_error>

#ifdef __ANDROID_API__
extern "C" void*  __mmap2(void*, size_t, int, int, int, size_t);
#define mymmap __mmap2
#define mmapoffset(x) ((x)>>12)
#else
#define mymmap mmap
#define mmapoffset(x) (x)
#endif

#define dbgprint(...)

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


// class for creating a memory mapped file.
// TODO: create internal sharedptr like I did in filehandle.
struct mappedmem {
    uint8_t *pmem;
    uint64_t phys_length;
    uint64_t dataofs;
    uint64_t length;

    static uint64_t round_up(uint64_t ofs, uint64_t base)
    {
        return ((ofs-1)|(base-1))+1;
    }
    static uint64_t round_down(uint64_t ofs, uint64_t base)
    {
        return ofs & ~(base-1);
    }


    mappedmem(mappedmem&& mm)
    {
        pmem = mm.pmem;
        phys_length = mm.phys_length;
        dataofs = mm.dataofs;
        length = mm.length;

        mm.pmem= nullptr;
    }
#ifndef _WIN32
    // maps offsets start..end from file.
    mappedmem(int f, uint64_t start, uint64_t end, int mmapmode= PROT_READ|PROT_WRITE)
    {
        uint64_t pagesize= std::max(0x1000, (int)sysconf(_SC_PAGE_SIZE));

        uint64_t phys_start= round_down(start, pagesize);
        uint64_t phys_end= round_up(end, pagesize);

        phys_length= phys_end - phys_start;
        length = end - start;
        dataofs= start - phys_start;

        if (phys_length==0) {
            pmem = NULL;
            return;
        }
        pmem= (uint8_t*)mymmap(NULL, phys_length, mmapmode, MAP_SHARED, f, mmapoffset(phys_start));
        if (pmem==MAP_FAILED)
            throw std::system_error(errno, std::generic_category(), "mmap");
    }

    // todo: add madvise support

    ~mappedmem()
    {
        if (!pmem)
            return;

        if (munmap(pmem, phys_length))
            perror("munmap");
    }
#endif
#ifdef _WIN32
    HANDLE m;
    int xlatmode2Protect(int mmode)
    {
        int r = 0;
        switch(mmode&(PROT_READ|PROT_WRITE))
        {
           case PROT_READ|PROT_WRITE: r |= PAGE_READWRITE; break;
           case PROT_READ: r |= PAGE_READONLY; break;
        }
        return r;
    }
    int xlatmode2Access(int mmode)
    {
        int r = 0;
        if (mmode&PROT_READ)
            r |= FILE_MAP_READ;
        if (mmode&PROT_WRITE)
            r |= FILE_MAP_WRITE;
        return r;
    }
    static uint64_t getpagesize()
    {
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwAllocationGranularity;
    }

    mappedmem(int f, uint64_t start, uint64_t end, int mmapmode= PROT_READ|PROT_WRITE)
        : m(INVALID_HANDLE_VALUE), pmem(NULL)
    {
        uint64_t pagesize= std::max(0x1000, (int)getpagesize());

        uint64_t phys_start= round_down(start, pagesize);
        uint64_t phys_end= round_up(end, pagesize);

        phys_length= phys_end - phys_start;
        length = end - start;
        dataofs= start - phys_start;

        if (phys_length==0) {
            pmem = NULL;
            return;
        }

        HANDLE fh=INVALID_HANDLE_VALUE;// note: both indicator value for anon-mapping, and error from get_osfhandle
        if (f!=-1) {
            fh = (HANDLE)_get_osfhandle(f);
            if (fh==INVALID_HANDLE_VALUE)
                throw std::system_error(errno, std::generic_category(), "invalid filehandle");
        }
        m = CreateFileMapping(fh, NULL, xlatmode2Protect(mmapmode), (phys_end-phys_start)>>32, (phys_end-phys_start)&0xFFFFFFFF, NULL);
        if (m==NULL)
            throw std::system_error(errno, std::generic_category(), "CreateFileMapping");
        pmem = (uint8_t*)MapViewOfFile(m, xlatmode2Access(mmapmode), phys_start>>32, phys_start&0xFFFFFFFF, phys_end-phys_start);
        if (pmem==NULL)
            throw std::system_error(errno, std::generic_category(), "MapViewOfFile");
    }
    ~mappedmem()
    {
        if (pmem)
            if (!UnmapViewOfFile(pmem))
                dbgprint("Error in UnmapViewOfFile: %08x\n", GetLastError());
        if (m)
            if (!CloseHandle(m))
                dbgprint("Error in CloseHandle: %08x\n", GetLastError());
    }
#endif

    uint8_t *data() { return pmem+dataofs; }
    const uint8_t *data() const { return pmem+dataofs; }

    uint8_t *begin() { return data(); }
    uint8_t *end() { return data() + length; }
    const uint8_t *begin() const { return data(); }
    const uint8_t *end() const { return data() + length; }
    size_t size()
    {
        return length;
    }
    uint8_t& operator[](size_t ix) { return data()[ix]; }

    // returns true when addresses stayed the same.
    bool resize(uint64_t newsize)
    {
#ifndef MREMAP_MAYMOVE
        // freebsd does not have mremap
        throw std::runtime_error("mmap.resize not supported");
#else
        uint64_t pagesize= std::max(0x1000, (int)sysconf(_SC_PAGE_SIZE));
        uint64_t new_physlength = round_up(dataofs+newsize, pagesize);

        uint8_t *newaddr = (uint8_t *)mremap(pmem, phys_length, new_physlength, MREMAP_MAYMOVE);
        if (newaddr==MAP_FAILED)
            throw std::system_error(errno, std::generic_category(), "mremap");

        bool havemoved = newaddr!=pmem;

        pmem = newaddr;
        phys_length = new_physlength;
        // dataofs should remain the same
        length = newsize;

        return !havemoved;
#endif
    }
};

