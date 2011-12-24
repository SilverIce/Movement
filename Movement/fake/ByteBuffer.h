#pragma once

#include "typedefs.h"
#include <assert.h>
#include <vector>

using Movement::int8;
using Movement::uint8;
using Movement::int16;
using Movement::uint16;
using Movement::int32;
using Movement::uint32;

using Movement::uint64;
using Movement::int64;

template<class T>
struct Unused
{
};

class ByteBuffer
{
public:

    ByteBuffer() : w_pos(0), r_pos(0) {}

    size_t rpos() const {return r_pos;}
    size_t wpos() const {return w_pos;}

    template<class T> ByteBuffer& operator << (const T& value)
    {
        append((const uint8*)&value, sizeof T);
        return *this;
    }

    template<class T> void put(size_t pos, const T& value)
    {
        put(pos, (const uint8*)&value, sizeof T);
    }

    void put(size_t pos, const uint8 *src, size_t cnt)
    {
        if(pos + cnt > size())
            assert(false);
        memcpy(&buffer[pos], src, cnt);
    }

    template<class T> void append (const T * array, size_t count)
    {
        append((const uint8*)array, count * (sizeof T));
    }

    void append (const uint8* src, size_t count)
    {
        if (!count)
            return;

        if (buffer.size() < w_pos + count)
            buffer.resize(w_pos + count);
        memcpy(&buffer[w_pos], src, count);
        w_pos += count;
    }

    template<class T> ByteBuffer& operator >> (T& value)
    {
        value = read<T>();
        return *this;
    }

    template<typename T> ByteBuffer& operator >> (Unused<T>)
    {
        read_skip<T>();
        return *this;
    }

    template <typename T> T read()
    {
        T r = read<T>(r_pos);
        r_pos += sizeof(T);
        return r;
    }

    template <typename T> T read(size_t pos) const
    {
        if (pos + sizeof(T) > size())
            assert(false);
            //throw ByteBufferException(false, pos, sizeof(T), size());
        T val = *((T const*)&buffer[pos]);
        return val;
    }

    void read(uint8 *dest, size_t len)
    {
        if (r_pos  + len > size())
            assert(false);
            //throw ByteBufferException(false, _rpos, len, size());
        memcpy(dest, &buffer[r_pos], len);
        r_pos += len;
    }

    void appendPackXYZ(float,float,float)
    {
        w_pos += 4;
    }

    bool empty() const { return buffer.empty();}

    template<class T> void read_skip()
    {
        r_pos += sizeof T;
    }

    size_t size() const { return buffer.size();}
private:

    size_t w_pos;
    size_t r_pos;
    std::vector<uint8> buffer;
};
