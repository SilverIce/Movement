
/**
  file:         ByteBufferExtensions.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "WorldPacket.h"
#include "typedefs.h"
#include "G3D/Vector3.h"
#include "G3D/Vector4.h"

namespace Movement
{
    inline ByteBuffer& operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
        return b;
    }

    inline ByteBuffer& operator << (ByteBuffer& b, const Vector4& v)
    {
        b << v.x << v.y << v.z << v.w;
        return b;
    }

    inline ByteBuffer& operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
        return b;
    }

    inline ByteBuffer& operator >> (ByteBuffer& b, Vector4& v)
    {
        b >> v.x >> v.y >> v.z >> v.w;
        return b;
    }
}
