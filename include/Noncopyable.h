#pragma once

class noncopyable
{
public:
    noncopyable(const noncopyable&) = default;
    noncopyable &operator=(const noncopyable&) = default;
protected:
    noncopyable() = default;
    ~noncopyable() = default;
};