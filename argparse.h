/* (C) 2003 XDA Developers  itsme@xs4all.nl
 *
 * $Header$
 */
#ifndef __ARGS_H__

#include <string>
#include <algorithm>
#include <stdlib.h>

#include "stringlibrary.h"

/*

    // cmd -apple  -a 1234   first  second  -

    for (auto& arg : ArgParser(argc, argv))
        switch (arg.option())
        {
            case 'a':
                if (arg.match("apple")) {
   
                }
                else {
                    a_arg = arg.getint();
                }
                break;
            case '-':
                if (arg.match("--long"))
                    a_long = true;
                break;
            case 0:
                usestdin = true;
                break;
            case -1:
                switch(n++)
                {
                case 0: first = arg.getstr(); break;
                case 1: second = arg.getstr(); break;
                }
        }

 */
class ArgParser {
    struct ArgIterator {
        const char **argv;  // argument list
        int argc;     // nr of argument strings
        int i;        // current argument
        const char *p;      // position in current argument
        const char *pend;   // end of current argument

        bool longmatch;
        bool isoption;

        void setcurrent()
        {
            longmatch = false;
            isoption = false;
            if (i==argc) {
                p = pend = nullptr;
                return;
            }
            p = argv[i];
            pend = p + stringlength(p);
        }

        ArgIterator(const char **argv, int i, int argc)
            : argv(argv), argc(argc), i(i)
        {
            setcurrent();
        }

        bool operator!=(const ArgIterator& rhs) const
        {
            return !(i==rhs.i && p == rhs.p);
        }

        ArgIterator& operator*() { return *this; }
        ArgIterator& operator++()
        {
            if (i==argc)
                throw std::range_error("arg out of range");

            i++;
            setcurrent();

            return *this;
        }

        /*
         * get single option character
         * returns -1 for non option arguments
         * returns 0  for single dash '-'
         */
        int option()
        {
            if (i==argc)
                throw std::range_error("arg out of range");
            if (*p=='-') {
                // a single '-' by itself is not considered an option,
                // so 'getstr()' will return "-"
                isoption = p[1]!=0;
                return p[1];
            }
            isoption = false;
            return -1;
        }

        /*
         * matches long option names
         * the name should include the leading option dash(es)
         *
         * like:  match("--verbose")
         *
         * the caller must make sure the matching order does not
         * lead to ambiguities.
         *
         * When the name matches, the current ptr will point after the match.
         */
        bool match(const char*name)
        {
            if (i==argc)
                throw std::range_error("arg out of range");
            int namelen = stringlength(name);
            if (namelen > stringlength(p))
                return false;
            if (stringcompare(name, p)==0) {
                p += namelen;
                longmatch = true;
                return true;
            }
            return false;
        }

        /*
         * checks for '--', usually used to end option processing
         */
        bool optionterminator()
        {
            if (i==argc)
                throw std::range_error("arg out of range");

            if (p+2!=pend)
                return false;

            return p[0]=='-' && p[1]=='-';
        }

        /*
         * gets the next argument string
         *
         * long arguments can have a '=' after the option name
         *
         *    --number=1234
         *
         */
        const char *getstr()
        {
            if (i==argc)
                throw std::range_error("arg out of range");
            if (isoption && !longmatch) {
                // skips '-' + optionchar
                p += 2;
            }
            else if (longmatch) {
                // long match need not match the full word.
                p = std::find(p, pend, '=');
                if (p!=pend)
                    p++;
            }

            if (*p == 0) {
                // next arg when at the end of a word
                operator++();
            }

            return p;
        }
        /*
         * return count for a repeated option:
         *
         *    -vvv   -> returns 3
         *
         */
        int count()
        {
            if (i==argc)
                throw std::range_error("arg out of range");
            if (longmatch)
                throw std::logic_error("can't count a --long option");
            char opt = p[1];
            const char *q = p+1;

            while (q!=pend && *q==opt)
                ++q;

            if (q!=pend)
                throw std::logic_error("counted options must all be equal");

            return pend - p - 1;
        }

        /*
         *  gets the integer after the current option.
         *  either in the same string, or in the next full argument.
         */
        int64_t getint()
        {
            getstr(); // ignoring result, we use 'p' directly.

            auto res = parsesigned(p, pend, 0);
            if (res.second != pend)
                throw std::logic_error("characters trailing number");
            return res.first;
        }
        uint64_t getuint()
        {
            getstr(); // ignoring result, we use 'p' directly.

            auto res = parseunsigned(p, pend, 0);
            if (res.second != pend)
                throw std::logic_error("characters trailing number");
            return res.first;
        }

    };

    int argc;
    const char **argv;
public:
    ArgParser(int argc, const char**argv)
        : argc(argc), argv(argv)
    {
    }


    ArgParser(int argc, char**argv)
        : argc(argc), argv(const_cast<const char**>(argv))
    {
    }

    auto begin() { return ArgIterator(argv, 1, argc); }
    auto end() { return ArgIterator(argv, argc, argc); }
};
#define __ARGS_H__
#endif
