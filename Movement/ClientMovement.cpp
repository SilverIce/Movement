#include "ClientMovement.h"
#include "WorldPacket.h"
#include "packet_builder.h"
//#include "WorldSession.h"
#include "UnitMovement.h"

#include <sstream>

namespace Movement
{
    class MovementMessage
    {
    public:
        typedef const UnitMovement* MessageSource;
    private:
        WorldPacket m_packet;
        MessageSource m_source;
        MSTime original_time;
        uint32 time_position;
    public:

        explicit MovementMessage(MessageSource source, uint16 opcode, size_t size) :
            m_packet(opcode, size), m_source(source), time_position(0)
        {}

        explicit MovementMessage(MessageSource source) : m_source(source), time_position(0) {}

        template<class T> void operator << (const T& value)
        {
            m_packet << value;
        }

        void operator << (const ClientMoveState& state)
        {
            original_time = state.ms_time;
            time_position = m_packet.wpos() + sizeof(uint32)/*UnitMoveFlag*/ + sizeof(uint16)/*UnitMoveFlag2*/;
            PacketBuilder::WriteClientStatus(state, m_packet);
        }

        MessageSource Source() const { return m_source;}
        const WorldPacket& Packet() const { return m_packet;}
        MSTime OrigTime() const { return original_time;}
        void CorrectTimeStamp(MSTime ms_time) { m_packet.put<uint32>(time_position,ms_time.time);}
    };

    void operator >> (ByteBuffer& data, ClientMoveState& state)
    {
        PacketBuilder::ReadClientStatus(state, data);
    }

    void Client::HandleOutcomingMessage(WorldPacket& recv_data)
    {
        if (!m_controlled)
            return;
        ObjectGuid guid;
        ClientMoveState state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state;

        state.ms_time = ClientToServerTime(state.ms_time);    // convert client ticks to server ticks
        m_controlled->m_moveEvents.QueueState(state);

        MovementMessage msg(m_controlled, recv_data.GetOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state;
        BroadcastMessage(msg);
    }

    /* For test. if set to true, enables old & wrong time correction */
    static bool old_way = false;
    static bool send_self = false;

    static MSTime timestamp_incr = 0;
    static MSTime timestamp_decr = 0;

    void Client::HandleIncomingMessage(MovementMessage& msg) const
    {
        if (msg.Source() == m_controlled && !send_self)
            return;

        // convert original time(server time) to local per-client time
        if (old_way == false)
            msg.CorrectTimeStamp(ServerToClientTime(msg.OrigTime()) + timestamp_incr - timestamp_decr);
        else
            msg.CorrectTimeStamp(ServerTime());
        SendPacket(msg.Packet());
    }

    void Client::CleanReferences()
    {
        if (m_controlled && m_controlled->client == this)
        {
            m_controlled->client = NULL;
        }
        m_controlled = NULL;
        m_local = NULL;
        m_socket = NULL;
    }

    void Client::Dereference(const UnitMovement * m)
    {
        if (m != m_controlled || m_controlled->client != this)
        {
            log_write("wtf?");
            return;
        }

        m_controlled = NULL;
    }

    void Client::SetControl(UnitMovement * newly_controlled)
    {
        if (m_controlled && m_controlled->client == this)
        {
            // Hm.. should i clean reference? what if it already in control of another client?
            m_controlled->client = NULL;
        }

        if (!m_local)
            m_local = newly_controlled;

        m_controlled = newly_controlled;
        m_controlled->client = this;
    }

    void Client::LostControl()
    {
        if (m_controlled && m_controlled->client == this)
        {
            m_controlled->client = NULL;
        }

        m_controlled = NULL;
    }

    Client::Client(HANDLE socket) :
        m_socket(socket),
        m_local(NULL),
        m_controlled(NULL)
    {
    }

    void Client::HandleTimeSyncResp(WorldPacket & recv_data)
    {
        uint32 counter = recv_data.read<uint32>();
        MSTime client_ticks = recv_data.read<uint32>();
        if (sync_counter.getCurrent() != counter)
        {
            log_write("Client::HandleTimeSyncResp: wrong counter value: %u and should be: %u", counter, sync_counter.getCurrent());
            return;
        }
        MSTime time_now = ServerTime();
        // client_ticks - client-side time when client receives TimeSyncResp message, need corrent it (take into account latency)
        //MSTime latency = uint32( (time_now - TimeSyncResp_sended).time / 2.f );
        m_time_diff = client_ticks - time_now;
    }

    void Client::_OnUpdate()
    {
        MSTime now = ServerTime();

        if ((now - m_last_sync_time).time > 10000)
        {
            m_last_sync_time = now;
            sync_counter.Increase();

            WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
            data << (uint32)sync_counter.getCurrent();
            SendPacket(data);
        }
    }

    void Client::HandleMoveTimeSkipped(WorldPacket & recv_data)
    {
        ObjectGuid guid;
        int32 time;
        recv_data >> guid.ReadAsPacked();
        recv_data >> time;
        //time_skipped = time;

        MovementMessage data(m_controlled, MSG_MOVE_TIME_SKIPPED, 16);
        data << guid.WriteAsPacked();
        data << time;
        BroadcastMessage(data);
    }

    std::string Client::ToString() const
    {
        std::stringstream str;
        str << "Server-side time: " << ServerTime().time << " Client-side time: " << ClientTime().time << std::endl;
        str << "Ack  counter: " << ack_counter.getCurrent() << std::endl;
        str << "Sync counter: " << sync_counter.getCurrent() << std::endl;
        return str.str();
    }
}
