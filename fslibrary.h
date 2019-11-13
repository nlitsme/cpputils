#pragma once
#include <numeric>
#include "stringlibrary.h"
#include <dirent.h>

#define dbgprint(...)
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
    int filter;  // bitmask of (1<<DT_nnn) values

    struct iter {
        pathvector path;
        RecursionType recurse;
        std::vector<DIR *> stack;

        dirent *cur;

        int filter;  // bitmask of (1<<DT_nnn) values

        iter(pathvector path, RecursionType recurse, int filter)
            : path(path), recurse(recurse), cur(nullptr), filter(filter)
        {
            dbgprint("iter.start\n");
        }
        iter()
            : recurse(DONE), cur(nullptr)
        {
            dbgprint("iter.default\n");
        }

        auto operator*()
        {
            dbgprint("deref\n");
            if (!cur && recurse!=DONE) {
                //push();
                cur = nextent();
            }
            if (!cur)
                throw std::runtime_error("eof");
            return std::make_tuple(path.join(cur->d_name), cur);
        }
        // return true for '.' and '..'
        bool isdirlink(dirent * ent)
        {
            char *p = ent->d_name;
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
            auto p = opendir(path.join().c_str());
            if (p==NULL) {
                dbgprint("ERROR in opendir: %s\n", strerror(errno));
                return false;
            }
            stack.push_back(p);
            dbgprint("opened (%d ; %d) dir %p\n", stack.size(), path.size(), p);

            return true;
        }
        void pop()
        {
            dbgprint("pop (%d ; %d)\n", stack.size(), path.size());
            if (-1==closedir(stack.back()))
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
        dirent *nextent()
        {
            dbgprint("nextent\n");
            while (!stack.empty()) {
                dirent *ent = readdir(stack.back());
                dbgprint("read dir -> %p\n", ent);
                if (!ent) {
                    dbgprint("ERROR in readdir: %s\n", strerror(errno));
                    pop();
                }
                else if (!isdirlink(ent))
                {
                    if (filter & (1<<ent->d_type))
                        return ent;

                    if (ent->d_type==DT_DIR) {
                        push(ent->d_name);
                    }
                }
            }
            return nullptr;
        }
        iter& operator++()
        {
            dbgprint("op++\n");
            cur = nextent();
            return *this;
        }
        bool operator!=(const iter& rhs)
        {
            dbgprint("op!=\n");
            if (recurse == DONE && rhs.recurse == DONE)
                return false;
            if (!cur) {
                push();
                cur = nextent();
            }
            return cur != rhs.cur || recurse != rhs.recurse;
        }
    };

    fileenumerator(const std::string& path, int filter=(1<<DT_REG), RecursionType recurse=SINGLE)
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
