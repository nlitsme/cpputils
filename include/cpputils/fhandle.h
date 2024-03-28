#pragma once
#include <memory>
#include <vector>
#include <span>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
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
#include <fcntl.h>

// wrap posix file handle in a c++ class,
// so it will be closed when it leaves the current scope.
// This is most useful for making code exception safe.
//
// `filehandle` can be assigned to, copied, deleted safely.

struct filehandle {

    // the 'ptr' class owns the filehandle,
    // the 'filehandle' has a shared_ptr to the 'ptr',
    // this makes sure the object can be copied as needed.
    struct ptr {
        int fh = -1;
        ptr(int fh)
            : fh(fh)
        {
        }
        ~ptr() { close(); }

        bool empty() const { return fh==-1; }

        void close()
        {
            if (fh==-1)
                return;
            if (-1 == ::close(fh)) {
                fh = -1;
                throw std::system_error(errno, std::generic_category(), "close");
            }
            fh = -1;
        }
    };
    std::shared_ptr<ptr> _p;


    filehandle() { }

    // the copy constructor is implemented in terms of the assignment operator.
    filehandle(const filehandle& fh)
    {
        *this = fh;
    }

    // construct with `int` is implemented using `assign int`.
    filehandle(int fh)
    {
        *this = fh;
    }

    // construct with path + flags opens a filesystem file.
    filehandle(const std::string& filename, int openflags = O_RDONLY, int mode=0666)
    {
        open(filename, openflags, mode);
    }

    void open(const std::string& filename, int openflags = O_RDONLY, int mode=0666)
    {
        *this = ::open(filename.c_str(), openflags, mode);
    }

    // discards and optionally closes any handle currently retained,
    // before creating a new wrapped filehandle.
    filehandle& operator=(int fh)
    { 
        if (fh==-1)
            throw std::system_error(errno, std::generic_category());
        _p = std::make_shared<ptr>(fh);
        return *this;
    }
    // copies the wrapped handle from the passed handle.
    filehandle& operator=(const filehandle& fh)
    {
        _p = fh._p;
        return *this;
    }
    bool empty() const { return !_p || _p->empty(); }

    // boolean-context checks if the filehandle has been set.
    operator bool () const { return !empty(); }


    // int-context returns the filehandle, so it can be passed to posix functions.
    operator int () const { return fh(); }
    int fh() const {
        if (!_p) throw std::runtime_error("no filehandle set");
        return _p->fh;
    }

    // explicitly close the filehandle.
    // note: other copies will note an exception when using the handle.
    void close() {
        if (!_p) throw std::runtime_error("no filehandle set");
        _p->close();
    }

    // for regular-files, returns the filesize,
    // for block-devices, returns the device size.
    uint64_t size()
    {
        struct stat st;
        if (fstat(fh(), &st))
            throw std::system_error(errno, std::generic_category(), "fstat");

        if (st.st_mode&S_IFREG)
            return st.st_size;
#ifndef _WIN32
        else if (st.st_mode&S_IFBLK) {
#ifdef DKIOCGETBLOCKCOUNT
            uint64_t bkcount;
            uint32_t bksize;
            if (-1==ioctl(fh(), DKIOCGETBLOCKCOUNT, &bkcount))
                throw std::system_error(errno, std::generic_category(), "ioctl(DKIOCGETBLOCKCOUNT)");
            if (-1==ioctl(fh(), DKIOCGETBLOCKSIZE, &bksize))
                throw std::system_error(errno, std::generic_category(), "ioctl(DKIOCGETBLOCKSIZE)");
            return bkcount*bksize;
#elif defined(BLKGETSIZE64)
            uint64_t devsize;
            if (-1==ioctl(fh(), BLKGETSIZE64, &devsize))
                throw std::system_error(errno, std::generic_category(), "ioctl(BLKGETSIZE64)");
            return devsize;
#else
            throw std::runtime_error("size not implemented for block dev");
#endif
        }
#endif
        else {
            // pipe or socket
            throw std::runtime_error("size not implemented for pipe or socket");
        }
    }

    // note: this changes the process-global, OS maintained file pointer.
    uint64_t seek(int64_t pos, int whence=SEEK_SET)
    {
        auto rc = ::lseek(fh(), pos, whence);
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "lseek");
        return rc;
    }
    // note: this returns the process-global, OS maintained file pointer.
    uint64_t tell()
    {
        return seek(0, SEEK_CUR);
    }

    // modify the file's size.
    void trunc(uint64_t pos)
    {
#ifndef _WIN32
        if (-1==::ftruncate(fh(), pos))
            throw std::system_error(errno, std::generic_category(), "truncate");
#else
        HANDLE msfh = (HANDLE)_get_osfhandle(fh());
        LARGE_INTEGER cur;
        // save filepos
        LARGE_INTEGER liZero; liZero.QuadPart = 0;
        if (!SetFilePointerEx(msfh, liZero, &cur, FILE_CURRENT))
            throw std::system_error(errno, std::generic_category(), "SetFilePointerEx");

        // change file size
        LARGE_INTEGER liPos; liPos.QuadPart = pos;
        if (!SetFilePointerEx(msfh, liPos, NULL, FILE_BEGIN))
            throw std::system_error(errno, std::generic_category(), "SetFilePointerEx");
        if (!SetEndOfFile(msfh))
            throw std::system_error(errno, std::generic_category(), "SetEndOfFile");

        // restore filepos
        if (!SetFilePointerEx(msfh, cur, NULL, FILE_BEGIN))
            throw std::system_error(errno, std::generic_category(), "SetFilePointerEx");
#endif
    }

    // ============================= write =============================
    // write    (PTR ptr, size_t count) -> size_t
    // write    (const RANGE& r) -> size_t
    // write    (PTR first, PTR last) -> size_t
    // pwrite   (uint64_t ofs, PTR ptr, size_t count) -> size_t

    template<typename PTR>
    auto write(PTR ptr, size_t count)
    {
        auto rc = ::write(fh(), ptr, count*sizeof(*ptr));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "write");
        
        return rc/sizeof(*ptr);
    }

    template<typename RANGE>
    auto write(const RANGE& r)
    {
        return write(&*r.begin(), r.size());
    }
    template<typename PTR>
    auto write(PTR first, PTR last)
    {
        return write(first, std::distance(first, last));
    }

    // Write without using the process-global filepointer.
    // Use this when writing to different locations in the file from
    // different threads.
    // The user should still make sure it controls who is writing where.
    template<typename PTR>
    auto pwrite(uint64_t ofs, PTR ptr, size_t count)
    {
        auto rc = ::pwrite(fh(), ptr, count*sizeof(*ptr), ofs);
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "pwrite");
        
        return rc/sizeof(*ptr);
    }


    // ============================= read =============================
    // - read    (PTR ptr, size_t count) -> size_t
    // - read    (PTR first, PTR last) -> size_t
    // - read    (INT count) -> bytevector
    // - read    (RANGE r) -> size_t
    // - readspan(std::span<T> r) -> std::span<const T>
    // - pread   (uint64_t ofs, PTR ptr, size_t count) -> size_t

    template<typename PTR>
    size_t read(PTR ptr, size_t count)
    {
        auto rc = ::read(fh(), ptr, count*sizeof(*ptr));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "read");

        return rc/sizeof(*ptr);
    }

    template<typename PTR>
    size_t read(PTR first, PTR last)
    {
        return read(first, std::distance(first, last));
    }
//  template<typename RANGE>
//  std::enable_if_t<!std::is_scalar_v<RANGE>, size_t> read(RANGE r)
//  {
//      return read(&*r.begin(), r.size());
//  }
    template<typename INT>
    std::enable_if_t<std::is_scalar_v<INT>, std::vector<uint8_t>> read(INT count)
    {
        std::vector<uint8_t> data(count);
        auto rc = read(&data[0], data.size());
        data.resize(rc);

        return data;
    }

#if __cplusplus > 201703L
    // returns a span for the actually read bytes.
    std::span<uint8_t> readspan(std::span<uint8_t> r)
    {
        size_t n = read(r.data(), r.size());
        return r.first(n);
    }
    std::span<uint64_t> readspan(std::span<uint64_t> r)
    {
        size_t n = read(r.data(), r.size());
        return r.first(n);
    }
    template<typename T>
    std::span<T> readspan(std::span<T> r)
    {
        size_t n = read(r.data(), r.size());
        return r.first(n);
    }

#endif
    // Read without using the process-global filepointer.
    // Use this when reading from different locations in the file from
    // different threads.
    template<typename PTR>
    size_t pread(uint64_t ofs, PTR ptr, size_t count)
    {
        auto rc = ::pread(fh(), ptr, count*sizeof(*ptr), ofs);
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "pread");

        return rc/sizeof(*ptr);
    }

};
