#pragma once

#include <ctime>
#include <string>
#include <system_error>
#include <cpputils/formatter.h>
#ifndef _WIN32
#include <sys/time.h>
#endif

template<int DUMMY=0>
class timedelta_cls {
    int64_t _usec;
public:
    timedelta_cls()
        : _usec(0)
    {
    }
    timedelta_cls(int64_t usec)
        : _usec(usec)
    {
    }

    int64_t totalusec() const { return _usec; }
    int64_t totalmsec() const { return _usec/1000; }
    int64_t totalseconds() const { return _usec/1000/1000; }
    int64_t totalminutes() const { return _usec/1000/1000/60; }
    int64_t totalhours() const { return _usec/1000/1000/60/60; }
    int64_t totaldays() const { return _usec/1000000/60/60/24; }

    int usec() const { return _usec % 1000000; }
    int msec() const { return (_usec/1000) % 1000; }
    int seconds() const { return (_usec/1000000) % 60; }
    int minutes() const { return (_usec/1000000/60) % 60; }
    int hours() const { return (_usec/1000000/60/60) % 24; }

    bool iszero() const { return _usec == 0; }

    timedelta_cls& operator+=(timedelta_cls rhs)
    {
        _usec += rhs.totalusec();
        return *this;
    }
    timedelta_cls& operator-=(timedelta_cls rhs)
    {
        _usec -= rhs.totalusec();
        return *this;
    }
    friend std::ostream& operator<<(std::ostream& os, const timedelta_cls& dt)
    {
        return os << dt.totalseconds() << "." << std::setw(6) << std::setfill('0') << dt.usec();
    }
    friend timedelta_cls operator-(timedelta_cls lhs, timedelta_cls rhs)
    {
        return timedelta_cls(lhs.totalusec() - rhs.totalusec());
    }
    friend timedelta_cls operator+(timedelta_cls lhs, timedelta_cls rhs)
    {
        return timedelta_cls(lhs.totalusec() + rhs.totalusec());
    }

    friend bool operator==(timedelta_cls lhs, timedelta_cls rhs) { return lhs.totalusec() == rhs.totalusec(); }
    friend bool operator!=(timedelta_cls lhs, timedelta_cls rhs) { return !(lhs==rhs); }
    friend bool operator<(timedelta_cls lhs, timedelta_cls rhs) { return lhs.totalusec() < rhs.totalusec(); }
    friend bool operator>=(timedelta_cls lhs, timedelta_cls rhs) { return lhs.totalusec() >= rhs.totalusec(); }
    friend bool operator>(timedelta_cls lhs, timedelta_cls rhs) { return lhs.totalusec() > rhs.totalusec(); }
    friend bool operator<=(timedelta_cls lhs, timedelta_cls rhs) { return lhs.totalusec() <= rhs.totalusec(); }


};

template<int DUMMY=0>
class timepoint_cls {
#ifdef _WIN32
    FILETIME _tv;
#else
    struct timeval _tv;
#endif
public:
    timepoint_cls()
    {
#ifdef _WIN32
        _tv.dwLowDateTime = 0;
        _tv.dwHighDateTime = 0;
#else
        _tv.tv_sec = 0;
        _tv.tv_usec = 0;
#endif
    }
    void setnow()
    {
#ifdef _WIN32
        GetSystemTimePreciseAsFileTime(&_tv);
#else
        gettimeofday(&_tv, 0);
#endif
    }
    static timepoint_cls now()
    {
        timepoint_cls t;
        t.setnow();

        return t;
    }
    timepoint_cls(int64_t epochusec)
    {
        set_unixepochusec(epochusec);
    }
    void set_unixepochusec(uint64_t epochusec)
    {
#ifdef _WIN32
        // seconds since 1601-01-01 00:00:00
        // ft = (time_t * 10000000) + 116444736000000000
        // ft = (usec * 10) + 116444736000
        uint64_t ft = (epochusec * 10) + 116444736000;
        _tv.dwLowDateTime = (DWORD)ft;
        _tv.dwHighDateTime = DWORD(ft>>32);
#else
        // seconds since 1970-01-01 00:00:00
        _tv.tv_sec = epochusec / 1000000;
        _tv.tv_usec = epochusec % 1000000;
#endif
    }
#ifdef _WIN32
    uint64_t msvcepochusec() const
    {
        return ( uint64_t(_tv.dwHighDateTime) << 32 ) | _tv.dwLowDateTime;
    }
    uint64_t unixepochusec() const
    {
        return msvcepochusec()/10 - 11644473600;
    }
    bool empty() const { return _tv.dwHighDateTime == 0; }

    operator const FILETIME&() const
    {
        return _tv;
    }
#else
    uint64_t msvcepochusec() const
    {
        return (unixepochusec() * 10) + 116444736000;
    }
    uint64_t unixepochusec() const
    {
        return uint64_t(_tv.tv_sec)*1000000 + _tv.tv_usec;
    }

    bool empty() const { return _tv.tv_sec == 0; }

    operator const struct timeval&() const
    {
        return _tv;
    }
#endif
    auto time() const { return unixepochusec()/1000000; }
    auto usec() const { return unixepochusec()%1000000; }

    timepoint_cls& operator+=(timedelta_cls<DUMMY> rhs)
    {
        set_unixepochusec(unixepochusec() + rhs.totalusec());

        return *this;
    }
    timepoint_cls& operator-=(timedelta_cls<DUMMY> rhs)
    {
        set_unixepochusec(unixepochusec() - rhs.totalusec());

        return *this;
    }


    operator double() const
    {
        return unixepochusec() * 0.000001;
    }
    operator int64_t() const
    {
        return unixepochusec();
    }
    std::string iso() const
    {
        struct tm tm;
        time_t t = this->time();
        if (NULL == localtime_r(&t, &tm))
            throw std::system_error(errno, std::generic_category(), "localtime");

        return stringformat("%04d-%02d-%02dT%02d:%02d:%02d.%06d%+03d:%02d", 
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                this->usec(),
#ifdef _WIN32
                0, 0
#else
                tm.tm_gmtoff/3600, (tm.tm_gmtoff/60)%60
#endif
                );
    }
    std::string isoutc() const
    {
        struct tm tm;
        time_t t = this->time();
        if (NULL == gmtime_r(&t, &tm))
            throw std::system_error(errno, std::generic_category(), "gmtime");

        return stringformat("%04d-%02d-%02dT%02d:%02d:%02d.%06dZ", 
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                this->usec());
    }
    friend std::ostream& operator<<(std::ostream& os, const timepoint_cls& t)
    {
        return os << t.time() << "." << std::setw(6) << std::setfill('0') << t.usec();
    }
    friend timedelta_cls<DUMMY> operator-(const timepoint_cls& lhs, const timepoint_cls& rhs)
    {
        return timedelta_cls((int64_t)lhs - (int64_t)rhs);
    }
    friend timepoint_cls operator+(timepoint_cls lhs, const timedelta_cls<DUMMY> rhs)
    {
        lhs += rhs;
        return lhs;
    }
    friend timepoint_cls operator-(timepoint_cls lhs, const timedelta_cls<DUMMY> rhs)
    {
        lhs -= rhs;
        return lhs;
    }
    friend timepoint_cls operator+(const timedelta_cls<DUMMY> lhs, timepoint_cls rhs)
    {
        return rhs+lhs;
    }

    friend bool operator==(const timepoint_cls& lhs, const timepoint_cls& rhs) { return int64_t(lhs) == int64_t(rhs); }
    friend bool operator!=(const timepoint_cls& lhs, const timepoint_cls& rhs) { return !(lhs==rhs); }
    friend bool operator<(const timepoint_cls& lhs, const timepoint_cls& rhs) { return int64_t(lhs) < int64_t(rhs); }
    friend bool operator>=(const timepoint_cls& lhs, const timepoint_cls& rhs) { return int64_t(lhs) >= int64_t(rhs); }
    friend bool operator>(const timepoint_cls& lhs, const timepoint_cls& rhs) { return int64_t(lhs) > int64_t(rhs); }
    friend bool operator<=(const timepoint_cls& lhs, const timepoint_cls& rhs) { return int64_t(lhs) <= int64_t(rhs); }


};


using timepoint = timepoint_cls<>;
using timedelta = timedelta_cls<>;

