#pragma once

// temporary, fake implementation
class WorldPacket
{
public:

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
