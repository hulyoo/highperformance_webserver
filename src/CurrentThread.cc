#include "CurrentThread.h"

namespace CurrentThread
{
    thread_local int t_cacheTid = 0;
    thread_local char t_tidString[32];
    thread_local int t_tidStringLength;
};