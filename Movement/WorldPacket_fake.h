#pragma once

// temporary, fake implementation
class WorldPacket
{
public:

    void Initialize(uint16, size_t){}

    template<class T>
    WorldPacket& operator << (const T&)
    {
        return *this;
    }

    template<class T>
    void append(const T&)
    {
    }

    void appendPackXYZ(float,float,float)
    {
    }
};
