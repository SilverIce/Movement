#pragma once

namespace G3D
{
    class Vector3;
    class Vector4;
}

extern unsigned int getMSTime();

namespace Movement
{
    typedef signed char     int8;
    typedef unsigned char	uint8;
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

    template<class T> inline float ToSeconds(T t) { return 0.001f * t; }
    //template<class T> inline double ToSeconds(T t) { return 0.001000000047497451 * t; }

    using G3D::Vector3;
    using G3D::Vector4;
}
