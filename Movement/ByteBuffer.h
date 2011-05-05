#pragma once

#ifdef USE_FAKE_CLASSES

#include "typedefs.h"

using Movement::int8;
using Movement::uint8;
using Movement::int16;
using Movement::uint16;
using Movement::int32;
using Movement::uint32;

using Movement::uint64;
using Movement::int64;

class ByteBuffer
{
public:

    ByteBuffer() : w_pos(0), r_pos(0) {}

    size_t rpos() {return 0;}
    size_t wpos() {return 0;}
    void rpos(int) {}
    void wpos(int) {}

    template<class T>
    ByteBuffer& operator << (const T&)
    {
        w_pos += sizeof T;
        return *this;
    }

    template<class T>
    ByteBuffer& operator >> (T&)
    {
        r_pos += sizeof T;
        return *this;
    }

    template<class T>
    void append(const T&)
    {
        w_pos += sizeof T;
    }

    template<class T>
    void append(const T*, size_t c)
    {
        w_pos += c * sizeof T;
    }

    void appendPackXYZ(float,float,float)
    {
        w_pos += 4;
    }

    template<class T>
    void appendPackGUID(const T&)
    {
        w_pos += 8;
    }

    uint64 readPackGUID()
    {
        r_pos += 8;
        return 0;
    }

    bool empty() const { return true;}

    template<class T>
    ByteBuffer& read_skip()
    {
        r_pos += sizeof T;
        return *this;
    }

    size_t size() const { return 0;}

    size_t w_pos;
    size_t r_pos;
};

#endif
