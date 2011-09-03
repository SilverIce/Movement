
/**
  file:         MovementMessage.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    class UnitMovementImpl;
    struct ClientMoveState;

    class MovementMessage
    {
    public:
        typedef const UnitMovementImpl* MessageSource;
    private:
        WorldPacket m_packet;
        MessageSource m_source;
        MSTime original_time;
        uint32 time_position;
        enum {NO_TIMESTAMP = 0xFFFFFFFF};
    public:

        explicit MovementMessage(MessageSource source, uint16 opcode, size_t size) :
            m_packet(opcode, size), m_source(source), time_position(NO_TIMESTAMP)
        {}

        explicit MovementMessage(MessageSource source) : m_source(source), time_position(NO_TIMESTAMP) {}

        template<class T> void operator << (const T& value)
        {
            m_packet << value;
        }

        void operator << (const ClientMoveState& state)
        {
            original_time = state.ms_time;
            time_position = m_packet.wpos() + UnitMoveFlag::Size;
            PacketBuilder::WriteClientStatus(state, m_packet);
        }

        /* Message's source. It might be deleted: unsafe to call his functions, access to his fields */
        MessageSource Source() const { return m_source;}
        const WorldPacket& Packet() const { return m_packet;}
        MSTime OrigTime() const { return original_time;}
        void CorrectTimeStamp(MSTime ms_time)
        {
            if (time_position != NO_TIMESTAMP)
                m_packet.put<uint32>(time_position,ms_time.time);
        }
    };

    inline void operator >> (ByteBuffer& data, ClientMoveState& state)
    {
        PacketBuilder::ReadClientStatus(state, data);
    }

    inline void operator << (ByteBuffer& b, const Vector3& v)
    {
        b << v.x << v.y << v.z;
    }

    inline void operator << (ByteBuffer& b, const Location& v)
    {
        b << v.x << v.y << v.z << v.orientation;
    }

    inline void operator >> (ByteBuffer& b, Vector3& v)
    {
        b >> v.x >> v.y >> v.z;
    }

    inline void operator >> (ByteBuffer& b, Location& v)
    {
        b >> v.x >> v.y >> v.z >> v.orientation;
    }

    inline void operator >> (ByteBuffer& b, MSTime& v)
    {
        b >> v.time;
    }

    inline void operator << (ByteBuffer& b, const MSTime& v)
    {
        b << v.time;
    }

    inline void operator >> (ByteBuffer& b, UnitMoveFlag& v)
    {
        v.raw = uint64(b.read<uint32>()) | (uint64(b.read<uint16>()) << 32);
    }

    inline void operator << (ByteBuffer& b, const UnitMoveFlag& v)
    {
        b << (uint32)v.raw;
        b << (uint16)(v.raw >> 32);
    }
}
