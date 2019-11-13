#pragma once

#include <stdint.h>

// HiresTimer provides a usec resolution timer on both POSIX and win32 platforms
#ifdef _WIN32
#include <windows.h>
class HiresTimer {
    DWORD _tick;
public:
    HiresTimer()
    {
        reset();
    }
    int64_t lap()
    {
        DWORD now= GetTickCount();
        int64_t tdiff= now-_tick;
        _tick= now;
        return tdiff*1000;
    }
    void reset()
    {
        _tick= GetTickCount();
    }

    // in usec
    int64_t elapsed() const
    {
        return (int64_t)1000*msecelapsed();
    }

    // in msec
    int32_t msecelapsed() const
    {
        DWORD now= GetTickCount();
        int64_t tdiff= now-_tick;
        // note: this fails for delays more than a few weeks
        return (int32_t)tdiff;
    }
    static void usleep(int64_t t)
    {
        // note: this fails for delays more than a few weeks
        Sleep((DWORD)(t/1000));
    }
    uint64_t getstamp() const
    {
        return uint64_t(1000)*_tick;
    }
    static uint64_t stamp()
    {
        return HiresTimer().getstamp();
    }
    static uint32_t msecstamp()
    {
        return (uint32_t)(HiresTimer().getstamp()/1000);
    }

    static uint32_t unixstamp()
    {
        const uint64_t EPOCH_BIAS= 116444736000000000i64;
        const uint64_t HUNDREDNS = 10000000;
        SYSTEMTIME sTime;
        GetSystemTime(&sTime);

        FILETIME fTime;
        ULARGE_INTEGER  ll;
        //time_t tt=0;

        if ( !SystemTimeToFileTime( &sTime, &fTime ) ) 
            return 0;

        ll.LowPart=  fTime.dwLowDateTime;
        ll.HighPart= fTime.dwHighDateTime;
        return (uint32_t)((ll.QuadPart - EPOCH_BIAS)/HUNDREDNS);

    }
};
#else
#include <sys/time.h>
#include <unistd.h>     // for usleep
class HiresTimer {
    struct timeval _tv;
public:
    HiresTimer()
    {
        reset();
    }
    int64_t lap()
    {
        struct timeval now;
        gettimeofday(&now, 0);
        int64_t tdiff= ((int64_t)now.tv_sec-_tv.tv_sec)*1000000LL+((int64_t)now.tv_usec-_tv.tv_usec);
        _tv= now;
        return tdiff;
    }
    void reset()
    {
        gettimeofday(&_tv, 0);
    }
    int64_t elapsed() const
    {
        struct timeval now;
        gettimeofday(&now, 0);
        int64_t tdiff= ((int64_t)now.tv_sec-_tv.tv_sec)*1000000LL+((int64_t)now.tv_usec-_tv.tv_usec);
        return tdiff;
    }
    int32_t msecelapsed() const
    {
        return elapsed()/1000;
    }
    static void usleep(int64_t t)
    {
        ::usleep(t);
    }
    uint64_t getstamp() const
    {
        return _tv.tv_sec*1000000LL+_tv.tv_usec;
    }
    static uint64_t stamp()
    {
        return HiresTimer().getstamp();
    }
    static uint32_t msecstamp()
    {
        return HiresTimer().getstamp()/1000;
    }
    static uint32_t unixstamp()
    {
        return time(0);
    }
};
#endif

