#pragma once
#include <unistd.h>
#include <sys/stat.h>

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
    size_t size()
    {
        struct stat st;
        if (-1==fstat(f, &st)) {
            perror("stat");
            throw std::runtime_error("stat");
        }
        return st.st_size;
    }
};


