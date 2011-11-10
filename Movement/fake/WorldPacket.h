#pragma once

#include "ByteBuffer.h"

class WorldPacket : public ByteBuffer
{
public:
    WorldPacket(unsigned short, size_t){}
    WorldPacket(){}

    void Initialize(unsigned short, size_t){}
    void SetOpcode(uint16){}
    uint16 GetOpcode() const { return 0;}
};
