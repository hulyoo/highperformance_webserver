#pragma once

#include <string>

class TimeStamp
{
public:
    explicit TimeStamp()
        : microSecondSinceEpoch_(0) 
    {}
    
    explicit TimeStamp(int64_t microSecondsSinceEpochArg)
        : microSecondSinceEpoch_(microSecondsSinceEpochArg)
    {}
    std::string toString() const;
    std::string toFormatString(bool showMicroSeconds = true) const;
    bool valid() const {return microSecondSinceEpoch_ > 0;}

    int microSecondSinceEpoch() const {return microSecondSinceEpoch_;}
    static TimeStamp now();
    static TimeStamp invalid()
    {
        return TimeStamp();
    }
    static const int kMicroSecondsPerSecond = 1000  * 1000;
private:
    int microSecondSinceEpoch_;
};

inline TimeStamp addTime(TimeStamp timeStamp,double seconds)
{
    int64_t delta = static_cast<int64_t>(seconds * TimeStamp::kMicroSecondsPerSecond);
    return TimeStamp(timeStamp.microSecondSinceEpoch() + delta);
}

inline bool operator<(TimeStamp lhs,TimeStamp rhs)
{
    return lhs.microSecondSinceEpoch() < rhs.microSecondSinceEpoch();
}

inline bool operator==(TimeStamp lhs,TimeStamp rhs)
{
    return lhs.microSecondSinceEpoch() == rhs.microSecondSinceEpoch();
}