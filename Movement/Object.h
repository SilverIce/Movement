#pragma once

#ifdef USE_FAKE_CLASSES

class WorldPacket;
class ByteBuffer;
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





