#pragma once

#include "typedefs.h"

namespace Movement
{
    struct MSTime
    {
    private:
        static uint32 decrease(const uint32& value, const uint32& decrement)
        {
            if (decrement > value)
                return (0xFFFFFFFF - decrement) + value;
            else
                return value - decrement;
        }

        static uint32 increase(const uint32& value, const uint32& increment)
        {
            if (value < (0xFFFFFFFF - increment))
                return value + increment;
            else
                return increment - (0xFFFFFFFF - value);
        }

    public:
        uint32 time;
    
        MSTime(const uint32& t) : time(t) {}
        MSTime() : time(0) {}

        MSTime operator + (const MSTime& t) const { return MSTime(increase(time,t.time));}
        MSTime operator - (const MSTime& t) const { return MSTime(decrease(time,t.time));}

        void operator = (const MSTime& t) { time = t.time;}
        void operator = (const uint32& t) { time = t;}
        void operator += (const MSTime& t) { *this = *this + t;}
        void operator -= (const MSTime& t) { *this = *this - t;}

        bool operator != (const MSTime& t) const { return time != t.time;}
        bool operator == (const MSTime& t) const { return time == t.time;}

        bool operator < (const MSTime& t) const { return time < t.time;}
        bool operator > (const MSTime& t) const { return time > t.time;}
        bool operator <= (const MSTime& t) const { return time <= t.time;}
        bool operator >= (const MSTime& t) const { return time >= t.time;}
    };
}
