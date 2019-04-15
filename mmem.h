#pragma once
// class wrappong a memmapped object

#include <sys/mman.h>
#include <unistd.h>

#ifdef __ANDROID_API__
extern "C" void*  __mmap2(void*, size_t, int, int, int, size_t);
#define mymmap __mmap2
#define mmapoffset(x) (x>>12)
#else
#define mymmap mmap
#define mmapoffset(x) (x)
#endif

// class for creating a memory mapped file.
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
        pmem= mm.pmem;
        phys_length= mm.phys_length;
        dataofs= mm.dataofs;

        mm.pmem= nullptr;
    }
    mappedmem(int f, uint64_t start, uint64_t end, int mmapmode= PROT_READ|PROT_WRITE)
    {
        uint64_t pagesize= std::max(0x1000, (int)sysconf(_SC_PAGE_SIZE));

        uint64_t phys_start= round_down(start, pagesize);
        uint64_t phys_end= round_up(end, pagesize);

        phys_length= phys_end - phys_start;
        length = end - start;

        pmem= (uint8_t*)mymmap(NULL, phys_length, mmapmode, MAP_SHARED, f, mmapoffset(phys_start));
        if (pmem==MAP_FAILED) {
            //printf("l=%llx, mm=%x, s=%llx\n", phys_length, mmapmode, phys_start);
            //printf("start=%llx -> %llx,   end=%llx -> %llx\n", start, phys_start, end, phys_end);
            perror("mmap");
            throw std::runtime_error("mmap");
        }

        dataofs= start - phys_start;
    }

    ~mappedmem()
    {
        if (!pmem)
            return;

        if (munmap(pmem, phys_length))
            perror("munmap");
    }
    uint8_t *ptr()
    {
        return pmem+dataofs;
    }

    uint8_t *begin()
    {
        return ptr();
    }
    uint8_t *end()
    {
        return ptr() + length;
    }
    size_t size()
    {
        return length;
    }
};

