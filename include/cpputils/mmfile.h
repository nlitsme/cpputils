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
class mappedfile {
    filehandle _f;
    mappedmem _m;
public:
    mappedfile(filehandle fh, int mmapmode)
        : _f(fh),
        _m(_f, 0, _f.size(), mmapmode)
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


    auto size() { return _m.size(); }
    auto begin() { return _m.begin(); }
    auto end() { return _m.end(); }

    // return true if pointers did not move.
    bool resize(uint64_t newsize)
    {
        _f.trunc(newsize);
        return _m.resize(newsize);
    }
};

