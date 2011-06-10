
/**
  file:         Session.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#include "ClientMoveStatus.h"
#include "WorldPacket.h"
#include "packet_builder.h"

extern void BroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg, WorldObject const* skipped);

namespace Movement
{
    class UnitMovement;

    struct MovementMessage
    {
    private:
        WorldPacket m_packet;
        uint32 time_position;
    public:
        uint32 original_time;

        explicit MovementMessage(uint16 opcode, size_t size) :
            m_packet(opcode, size), time_position(0), original_time(0)
        {}

        explicit MovementMessage() : time_position(0), original_time(0) {}

        void Initialize(uint16 opcode, size_t size)
        {
            m_packet.Initialize(opcode,size);
            time_position = 0;
            original_time = 0;
        }

        template<class T> void operator << (const T& value)
        {
            m_packet << value;
        }

        void operator << (const ClientMoveState& state)
        {
            time_position = m_packet.wpos() + 6;
            PacketBuilder::WriteClientStatus(state, m_packet);
        }

        const WorldPacket& Packet() const { return m_packet;}
        void CorrectTimeStamp(MSTime ms_time) { m_packet.put<uint32>(time_position,ms_time.time);}
    };

    class Client
    {
        HANDLE m_socket;
        UnitMovement * m_local;
        UnitMovement * m_controlled;
        MSTime m_time_diff;             // difference between client and server time: diff = client_ticks - server_ticks
        MSTime m_last_sync_time;
        UInt32Counter ack_counter;
        UInt32Counter sync_counter;
        uint32 last_recvd_ack;
        int32 time_skipped;

        static int32 timestamp_incr;
        static MSTime ServerTime() { return MSTime(getMSTime());}
        MSTime ServerToClientTime(uint32 server_time) const { return MSTime(server_time) + m_time_diff + MSTime(timestamp_incr);}
        MSTime ClientTime() const {return ServerToClientTime(ServerTime().time);}
        MSTime ClientToServerTime(uint32 client_time) const { return MSTime(client_time) - m_time_diff;}

        void BroadcastMessage(MovementMessage& msg) const { BroadcastMessage(m_controlled->Owner, msg, m_local->Owner);}

    public:

        /** Client's lifetime bounded to WorldSession lifetime */
        Client(HANDLE socket);

        ~Client()
        {
            CleanReferences();
        }

        void Dereference(const UnitMovement * m);

        void LostControl();
        //UnitMovement* Controlled() const { return m_controlled;}
        void SetControl(UnitMovement * mov);

        void CleanReferences();

        void _OnUpdate();

        void HandleIncomingMoveState(ByteBuffer& data);

        /**    Handles messages from another clients */
        void HandleIncomingMessage(MovementMessage& msg) const;
        /**    Handles messages from that client */
        void HandleOutcomingMessage(WorldPacket& recv_data, MovementMessage& msg);

        void HandleTimeSyncResp(WorldPacket& recv_data);

        void HandleMoveTimeSkipped(WorldPacket & recv_data);
    };
}
