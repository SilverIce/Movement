#pragma once

#ifdef USE_FAKE_CLASSES

class ByteBuffer
{
public:

    ByteBuffer() : w_pos(0), r_pos(0) {}

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
