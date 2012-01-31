#pragma once

#include "framework/typedefs.h"

namespace Movement
{
    struct MSTime
    {
    private:
        enum{
            UInt32Max = uint32(0xFFFFFFFF),
        };
        static uint32 decrease(const uint32& value, const uint32& decrement)
        {
            if (decrement > value)
                return (UInt32Max - decrement) + value;
            else
                return value - decrement;
        }

        static uint32 increase(const uint32& value, const uint32& increment)
        {
            if (value < (UInt32Max - increment))
                return value + increment;
            else
                return increment - (UInt32Max - value);
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
