#pragma once

#include "ByteBuffer.h"
#include "Client.h"

namespace Movement
{
    class Packet : public ByteBuffer
    {
        uint16 m_opcode;
    public:
        Packet(uint16 opcode, size_t size) : ByteBuffer(size), m_opcode(opcode) {
        }

        Packet() : m_opcode(0) {}

        explicit Packet(const PacketData& data) : m_opcode(data.opcode) {
            append(data.bytes, data.byteCount);
        }

        PacketData toPacketData() const {
            PacketData data(GetOpcode(),(const uint8*)contents(),size());
            return data;
        }

        void SetOpcode(uint16 opcode) { m_opcode = opcode;}

        uint16 GetOpcode() const { return m_opcode;}
    };
}
