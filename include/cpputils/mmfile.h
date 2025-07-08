#pragma once

#include <fcntl.h>

#include <cpputils/mmem.h>
#include <cpputils/fhandle.h>

/*
   retain both the filehandle and the mmap

Note: this object may be quite unnescesary, since the mmap manual states:
   After the mmap() call has returned, the file descriptor, fd, can be closed
   immediately without invalidating the mapping.

 */
// todo: make this copyable.
// todo: add span
class mappedfile {
    filehandle _f;
    mappedmem _m;
public:
    mappedfile(filehandle fh, int mmapmode)
        : _f(fh), _m(_f, 0, _f.size(), mmapmode)
    {
    }

    mappedfile(const std::string& filename, int openflags = O_RDONLY, int mode=0666)
        : mappedfile(open(filename.c_str(), openflags, mode), openflags==O_RDONLY ? PROT_READ : (PROT_READ|PROT_WRITE))
    {
    }
    mappedfile(int fh, int mmapmode = PROT_READ)
        : mappedfile(filehandle{fh}, mmapmode)
    {
    }
    auto file() { return _f; }

    uint8_t *data() { return _m.data(); }
    const uint8_t *data() const { return _m.data(); }
    uint8_t *begin() { return _m.begin(); }
    uint8_t *end() { return _m.end(); }
    const uint8_t *begin() const { return _m.begin(); }
    const uint8_t *end() const { return _m.end(); }
    size_t size() { return _m.size(); }
    uint8_t& operator[](size_t ix) { return _m[ix]; }


    // return true if pointers did not move.
    bool resize(uint64_t newsize)
    {
        _f.trunc(newsize);
        return _m.resize(newsize);
    }
};

