
/**
  file:         typedefs.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

// do not commit that assert redefinition!
#include "G3D/debugAssert.h"
#define mov_assert(exp) alwaysAssertM(exp, "")

namespace G3D
{
    class Vector3;
    class Vector4;
}

extern unsigned int getMSTime();

namespace Movement
{
    using G3D::Vector3;
    using G3D::Vector4;

    typedef signed char     int8;
    typedef unsigned char   uint8;
    typedef short           int16;
    typedef unsigned short  uint16;
    typedef int             int32;
    typedef unsigned int    uint32;

    typedef __int64         int64;
    typedef unsigned __int64 uint64;

    // TODO: move it out of here
    inline uint32 getMSTimeDiff(uint32 old_time, uint32 new_time)
    {
        if (old_time > new_time)
            return (0xFFFFFFFF - old_time) + new_time;
        else
            return new_time - old_time;
    }

    inline uint32 SecToMS(float sec)
    {
        return static_cast<uint32>(sec * 1000.f);
    }

    inline float MSToSec(uint32 ms)
    {
        return ms / 1000.f;
    }

    extern void log_write(const char* fmt, ...);
    extern void log_console(const char* str, ...);

    template<typename T, size_t N>
    inline size_t CountOf(const T (&t)[N])
    {
        return N;
    }

    template<bool> struct _assert_fail;
    template<> struct _assert_fail<true> { enum{value = 1}; };

    #define static_assert(B)\
        enum {\
            static_assert_enum##__LINE__ = sizeof(_assert_fail<(bool)(B)>)\
        }

    /// Declarations:
    struct UpdaterLink;

    template<class T> class LinkedListElement;
    template<class T> class LinkedList;
    typedef LinkedList<UpdaterLink> MovementBaseList;
    typedef LinkedListElement<UpdaterLink> MovementBaseLink;
}
