
/**
  file:         ByteBufferExtensions.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "WorldPacket.h"
#include "typedefs.h"
#include "Location.h"

namespace Movement
{
    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator << (ByteBuffer& b, const Location& v)
    {
        b << v.x << v.y << v.z << v.orientation;
    }

    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    inline void operator >> (ByteBuffer& b, Location& v)
    {
        b >> v.x >> v.y >> v.z >> v.orientation;
    }

    inline void operator >> (ByteBuffer& b, MSTime& v)
    {
        b >> v.time;
    }

    inline void operator << (ByteBuffer& b, const MSTime& v)
    {
        b << v.time;
    }
}
