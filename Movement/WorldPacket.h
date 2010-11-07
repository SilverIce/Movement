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

};


class WorldPacket : public ByteBuffer
{
public:

    void Initialize(unsigned short, size_t){}


    template<class T>
    void append(const T&)
    {
    }

    void appendPackXYZ(float,float,float)
    {
    }
};
#endif






