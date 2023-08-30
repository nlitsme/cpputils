#pragma once
#include <numeric>
#include <tuple>
#include <cpputils/stringlibrary.h>
#ifndef _WIN32
#include <dirent.h>
#endif
#ifdef _WIN32
#include <windows.h>
#endif
#include <cpputils/formatter.h>

#define dbgprint(...)
//#define dbgprint print
/*
 * Usage:
 *
 *  for (auto [fn, ent] : fileenumerator(path))
 *     handlefile(fn)
 *
 *  for (auto [fn, ent] : fileenumerator(path, 1<<DT_DIR))
 *     handledir(fn)
 *
 */

struct pathvector {
    std::vector<std::string> _v;

    pathvector()
    {
    }
    pathvector(const std::string& path, const std::string& sep = "/\\")
    {
        for (auto s : stringsplitter(path, sep))
            _v.emplace_back(s);
    }
    void push_back(const std::string& item)
    {
        _v.emplace_back(item);
    }
    void pop_back()
    {
        _v.pop_back();
    }
    auto size()
    {
        return _v.size();
    }
    std::string join() const
    {
        if (_v.empty())
            return {};
        return std::accumulate(_v.begin()+1, _v.end(), *_v.begin(), [](auto a, auto b) { return a + std::string("/") + b; });
    }
    std::string join(const std::string& name) const
    {
        return join() + "/" + name;
    }
};
/*
 *  TODO: move opendir/readdir functions to 'filesystem' object,
 *  so i can use this with a mock filesystem in a unittest.
 *
 *  TODO: implement search strategies
 *
 *  TODO: implement directory filter.
 *
 *  TODO: add flag specifying which entries to yield.
 *
 *  TODO: yield struct with path/filename + dirent instead of filename.
 */
struct fileenumerator {
    pathvector path;

    enum RecursionType {
        DONE,
        SINGLE,
        BREADTHFIRST,
        DEPTHFIRST,
    };
    RecursionType recurse;
    enum FilterType {
        FILT_DIRECTORY=1,
        FILT_REGULAR=2,
        FILT_SYMLINK=4,
    };
    int filter;  // bitmask of FILT_xxx values

#ifndef _WIN32
    struct fileent {
        dirent *ent = nullptr;
        bool empty() const { return ent==nullptr; }
        std::string name() const { return ent->d_name; }
        bool isdir() const { return ent->d_type == DT_DIR; }
        bool isfile() const { return ent->d_type == DT_REG; }
        bool match(int filter) const { return filter & (1<<ent->d_type); }
        // return true for '.' and '..'
        bool isdirlink() const
        {
            const char *p = ent->d_name;
            char c = *p++;
            if (c != '.')
                return false;
            c = *p++;
            if (c == 0)
                return true;
            if (c != '.')
                return false;
            c = *p++;
            return c == 0;
        }
        friend std::ostream& operator<<(std::ostream& os, const fileent& f)
        {
            if (f.empty())
                return os << "(null)";
            auto ent = f.ent;
            return os << stringformat("%10d/i %3d/l x%x/t '%s'", ent->d_ino, ent->d_reclen, ent->d_type, f.name());
        }
    };
    struct iter {
        pathvector path;
        RecursionType recurse;
        std::vector<DIR *> stack;  // stack of opendir handles

        fileent cur;

        int filter;  // bitmask of (1<<DT_nnn) values

        iter(pathvector path, RecursionType recurse, int filter)
            : path(path), recurse(recurse), filter(filter)
        {
            dbgprint("iter.start\n");
        }
        iter()
            : recurse(DONE)
        {
            dbgprint("iter.default\n");
        }

        auto operator*()
        {
            dbgprint("deref\n");
            if (cur.empty() && recurse!=DONE) {
                //push();
                nextent();
            }
            if (cur.empty())
                throw std::runtime_error("eof");
            return std::make_tuple(path.join(cur.name()), cur);
        }
        void push(const std::string& name)
        {
            dbgprint("push\n");
            if (!name.empty())
                path.push_back(name);
            if (!push())
                path.pop_back();
        }
        bool push()
        {
            dbgprint("push -> opendir: %s\n", path.join());
            auto p = ::opendir(path.join().c_str());
            if (p==NULL) {
                dbgprint("ERROR in opendir: %s\n", strerror(errno));
                return false;
            }
            stack.push_back(p);
            dbgprint("opened (%d ; %d) dir %p\n", stack.size(), path.size(), p);

            nextent();

            return true;
        }
        void pop()
        {
            dbgprint("pop (%d ; %d)\n", stack.size(), path.size());
            if (-1==::closedir(stack.back()))
                dbgprint("ERROR in closedir: %s\n", strerror(errno));
            dbgprint("closed dir %p\n", stack.back());
            stack.pop_back();
            path.pop_back();
            if (stack.empty())
            {
                recurse = DONE;
                dbgprint("DONE\n");
            }
        }
        bool nextent()
        {
            dbgprint("nextent\n");
            while (!stack.empty()) {
                cur.ent = ::readdir(stack.back());
                dbgprint("read dir(%d) -> %s\n", stack.size(), cur);
                if (!cur.ent) {
                    dbgprint("ERROR in readdir: %s\n", strerror(errno));
                    pop();
                }
                else if (!cur.isdirlink())
                {
                    if (cur.isdir())
                        push(cur.name());
                    //if (cur.match(filter))
                        return true;
                }
            }
            return false;
        }
        iter& operator++()
        {
            dbgprint("op++\n");
            nextent();
            return *this;
        }
        bool operator!=(const iter& rhs)
        {
            dbgprint("op!=\n");
            if (recurse == DONE && rhs.recurse == DONE)
                return false;
            if (cur.empty()) {
                push();
            }
            return cur.empty() != rhs.cur.empty() || recurse != rhs.recurse;
        }
    };
#endif
#ifdef _WIN32
    struct fileent {
        WIN32_FIND_DATA ent = {};
        bool empty() const { return ent.cFileName[0]==0; }
        std::string name() const { return ent.cFileName; }
        bool isdir() const { return ent.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY; }
        bool isfile() const { return ent.dwFileAttributes&FILE_ATTRIBUTE_NORMAL; }
        bool match(int filter) const { return filter & ent.dwFileAttributes; }
        // return true for '.' and '..'
        bool isdirlink() const
        {
            const char *p = ent.cFileName;
            char c = *p++;
            if (c != '.')
                return false;
            c = *p++;
            if (c == 0)
                return true;
            if (c != '.')
                return false;
            c = *p++;
            return c == 0;
        }
        friend std::ostream& operator<<(std::ostream& os, const fileent& f)
        {
            auto &ent = f.ent;
            auto cv = [](FILETIME ft) {
                uint64_t ftval = uint64_t(ft.dwHighDateTime)<<32 | ft.dwLowDateTime;
                return (ftval-116444736000000000)/10000000;
            };
            return os << stringformat("%04x/a %08x/c %08x/a %08x/m %08x:%08x/s %08x:%08x/? (%s) %s",
                    ent.dwFileAttributes,
                    cv(ent.ftCreationTime),
                    cv(ent.ftLastAccessTime),
                    cv(ent.ftLastWriteTime),
                    ent.nFileSizeHigh, ent.nFileSizeLow,
                    ent.dwReserved0, ent.dwReserved1,
                    ent.cAlternateFileName,
                    ent.cFileName);
        }
    };
    struct iter {
        pathvector path;
        RecursionType recurse;
        std::vector<HANDLE> stack;   // stack of open 'FindFirstFile' handles

        fileent cur;   // most recent 'dir enctry' result

        int filter;  // bitmask of (1<<DT_nnn) values

        iter(pathvector path, RecursionType recurse, int filter)
            : path(path), recurse(recurse), filter(filter)
        {
            dbgprint("iter.start\n");
        }
        iter()
            : recurse(DONE)
        {
            dbgprint("iter.default\n");
        }

        auto operator*()
        {
            dbgprint("deref\n");
            if (cur.empty() && recurse!=DONE) {
                //push();
                nextent();
            }
            if (cur.empty())
                throw std::runtime_error("eof");
            return std::make_tuple(path.join(cur.name()), cur);
        }
        void push(const std::string& name)
        {
            dbgprint("push\n");
            if (!name.empty())
                path.push_back(name);
            if (!push())
                path.pop_back();
        }
        bool push()
        {
            dbgprint("push -> FindFirstFile: %s\n", path.join());
            auto p = FindFirstFile(path.join("*.*").c_str(), &cur.ent);
            if (p==INVALID_HANDLE_VALUE) {
                auto e = GetLastError();
                if (e!=ERROR_NO_MORE_FILES)
                    dbgprint("ERROR in FindFirstFile: %08lx\n", e);

                return false;
            }
            stack.push_back(p);
            dbgprint("opened (%d ; %d) dir %p -> %s\n", stack.size(), path.size(), p, cur);

            return true;
        }
        void pop()
        {
            dbgprint("pop (%d ; %d)\n", stack.size(), path.size());
            if (!FindClose(stack.back()))
                dbgprint("ERROR in FindClose: %s\n", strerror(errno));
            dbgprint("closed dir %p\n", stack.back());
            stack.pop_back();
            path.pop_back();
            if (stack.empty())
            {
                recurse = DONE;
                dbgprint("DONE\n");
            }
        }
        bool nextent()
        {
            dbgprint("nextent\n");
            while (!stack.empty()) {
                if (!FindNextFile(stack.back(), &cur.ent))
                {
                    auto e = GetLastError();
                    if (e!=ERROR_NO_MORE_FILES)
                        dbgprint("ERROR in FindNextFile: %08lx\n", e);
                    pop();
                    continue;
                }
                dbgprint("next(%d) -> %s\n", stack.size(), cur);
                if (!cur.isdirlink())
                {
                    if (cur.isdir())
                        push(cur.name());
                    //if (cur.match(filter))
                        return true;
                }
            }
            return false;
        }
        iter& operator++()
        {
            dbgprint("op++\n");
            nextent();
            return *this;
        }
        bool operator!=(const iter& rhs)
        {
            dbgprint("op!=\n");
            if (recurse == DONE && rhs.recurse == DONE)
                return false;
            if (cur.empty()) {
                push();
            }
            return cur.empty() != rhs.cur.empty() || recurse != rhs.recurse;
        }
    };
#endif
    fileenumerator(const std::string& path, int filter=FILT_DIRECTORY, RecursionType recurse=SINGLE)
        : path{path}, recurse(recurse), filter(filter)
    {
    }
    iter begin() const
    {
        return iter(path, recurse, filter);
    }
    iter end() const
    {
        return iter{};
    }
};
