#pragma once

#include <string.h> //memset

inline void memZero(void* p,size_t n)
{
    memset(p,0,n);
}