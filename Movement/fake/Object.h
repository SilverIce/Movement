#pragma once

#include "typedefs.h"
#include "ByteBuffer.h"
#include "UpdateFields.h"
#include "ObjectGuid.h"

using Movement::int8;
using Movement::uint8;
using Movement::int16;
using Movement::uint16;
using Movement::int32;
using Movement::uint32;

using Movement::uint64;
using Movement::int64;


class WorldPacket;
class ByteBuffer;
class Player;

class WorldObject
{
public:

    void SetGuidValue(uint32, ObjectGuid) {}
    ObjectGuid GetGuidValue(uint32) const { return ObjectGuid();}
    ObjectGuid GetObjectGuid() const { return ObjectGuid();}

    const ByteBuffer& GetPackGUID() const { return m_packGuid; }

    float GetFloatValue(uint16) const { return 0.f;}
    void SetFloatValue(uint16,float)  {}

    float GetUInt64Value(uint16) const { return 0.f;}
    void SetUInt64Value(uint16,uint64)  {}

    uint64 GetGUID() const { return 0; }

    ByteBuffer m_packGuid;

    const char* GetName() const { return "<fake WorldObject>";}
};
