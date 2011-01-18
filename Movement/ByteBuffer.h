#pragma once

#ifdef USE_FAKE_CLASSES

class ByteBuffer
{
public:

    template<class T>
    ByteBuffer& operator << (const T&)
    {
        return *this;
    }

    template<class T>
    ByteBuffer& operator >> (T&)
    {
        return *this;
    }

    template<class T>
    void append(const T&)
    {
    }

    template<class T>
    void append(const T*, size_t)
    {
    }

    void appendPackXYZ(float,float,float)
    {
    }

    bool empty() const { return true;}

    template<class T>
    ByteBuffer& read_skip() { return *this;}
};

#endif