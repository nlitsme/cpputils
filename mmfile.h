#pragma once

#include <fcntl.h>

#include "mmem.h"
#include "fhandle.h"

/* retain both the filehandle and the mmap */
class mappedfile {
    filehandle _f;
    mappedmem _m;
public:
    mappedfile(filehandle fh, int flags = O_RDONLY)
        : _f(fh),
        _m(_f, 0, _f.size(), (flags==O_RDONLY)?PROT_READ:(flags==O_WRONLY)?PROT_WRITE:(flags==O_RDWR)?(PROT_READ|PROT_WRITE):0)
    {
    }

    mappedfile(const std::string& filename, int openflags = O_RDONLY, int mode=0666)
        : mappedfile(open(filename.c_str(), openflags, mode))
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
};

