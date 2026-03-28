#pragma once
#include <thread>

namespace CurrentThread
{
    extern thread_local int t_cacheTid;
    extern thread_local char t_tidString[32];
    extern thread_local int t_tidStringLegth;

    void cacheTid();

    inline int tid()
    {
        if(__builtin_expect(t_cacheTid == 0,0))
        {
            cacheTid();
        }
        return t_cacheTid;
    }

    inline const char* tidString()
    {
        return t_tidString;
    }

    inline int tidStringLegth()
    {
        return t_tidStringLegth;
    }
};