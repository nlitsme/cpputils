#pragma once

#include <sys/time.h>
#include <string>
#include <system_error>
#include <cpputils/formatter.h>


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
    struct timeval _tv;
public:
    timepoint_cls()
    {
        _tv.tv_sec = 0;
        _tv.tv_usec = 0;
    }
    void setnow()
    {
        gettimeofday(&_tv, 0);
    }
    static timepoint_cls now()
    {
        timepoint_cls t;
        t.setnow();

        return t;
    }
    timepoint_cls(int64_t epochusec)
    {
        _tv.tv_sec = epochusec / 1000000;
        _tv.tv_usec = epochusec % 1000000;
    }
    bool empty() const { return _tv.tv_sec == 0; }

    auto time() const { return _tv.tv_sec; }
    auto usec() const { return _tv.tv_usec; }

    operator const struct timeval&() const
    {
        return _tv;
    }

    timepoint_cls& operator+=(timedelta_cls<DUMMY> rhs)
    {
        int64_t us = _tv.tv_usec + rhs.totalusec();
        _tv.tv_usec = us % 1000000;
        _tv.tv_sec += us/1000000;

        return *this;
    }
    timepoint_cls& operator-=(timedelta_cls<DUMMY> rhs)
    {
        int64_t us = _tv.tv_usec - rhs.totalusec();
        _tv.tv_usec = us % 1000000;
        _tv.tv_sec += us/1000000;

        return *this;
    }


    operator double() const
    {
        return _tv.tv_sec + _tv.tv_usec * 0.000001;
    }
    operator int64_t() const
    {
        return _tv.tv_sec * 1000000LL + _tv.tv_usec;
    }
    std::string iso() const
    {
        struct tm tm;
        time_t t = _tv.tv_sec;
        if (NULL == localtime_r(&t, &tm))
            throw std::system_error(errno, std::generic_category(), "gmtime");

        return stringformat("%04d-%02d-%02dT%02d:%02d:%02d.%06d%+03d:%02d", 
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                _tv.tv_usec,
                tm.tm_gmtoff/3600, (tm.tm_gmtoff/60)%60);
    }
    std::string isoutc() const
    {
        struct tm tm;
        time_t t = _tv.tv_sec;
        if (NULL == gmtime_r(&t, &tm))
            throw std::system_error(errno, std::generic_category(), "gmtime");

        return stringformat("%04d-%02d-%02dT%02d:%02d:%02d.%06dZ", 
                tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec,
                _tv.tv_usec);
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

