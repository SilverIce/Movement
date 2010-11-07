#pragma once

#ifdef USE_FAKE_CLASSES

#include "WorldPacket.h"

class Player;

class WorldObject
{
public:

    const ByteBuffer& GetPackGUID() const { return m_packGuid; }

    void SendMessageToSet(WorldPacket *, bool){}
    void SendMessageToSetExcept(WorldPacket *, const Player*){}

    ByteBuffer m_packGuid;
};

#endif





