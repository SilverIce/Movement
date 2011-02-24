#pragma once

#include "ByteBuffer.h"

#ifdef USE_FAKE_CLASSES

class WorldPacket : public ByteBuffer
{
public:
    WorldPacket(unsigned short, size_t){}
    WorldPacket(){}

    void Initialize(unsigned short, size_t){}
};
#endif






