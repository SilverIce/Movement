#include "ClientMovement.h"
#include "packet_builder.h"
#include "WorldSession.h"
#include "UnitMovement.h"
#include "ObjectGuid.h"

void SendPacket(HANDLE socket, const WorldPacket& data)
{
    ((WorldSession*)socket)->SendPacket(&data);
}

namespace Movement
{
    void operator >> (ByteBuffer& data, ClientMoveState& state)
    {
        PacketBuilder::ReadClientStatus(state, data);
    }

    int32 Client::timestamp_incr = 5000;

    void Client::HandleOutcomingMessage(WorldPacket& recv_data)
    {
        if (!m_controlled)
            return;
        ObjectGuid guid;
        ClientMoveState state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state;

        state.ms_time = ClientToServerTime(state.ms_time).time;    // convert client ticks to server ticks
        m_controlled->m_moveEvents.QueueState(state);

        MovementMessage msg(recv_data.GetOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state;
        BroadcastMessage(msg);
    }

    /* For test. if set to true, enables old & wrong time correction */
    bool old_way = false;

    void Client::HandleIncomingMessage(MovementMessage& msg) const
    {
        // convert original time(server time) to local per-client time
        if (old_way == false)
            msg.CorrectTimeStamp(ServerToClientTime(msg.original_time) + MSTime(timestamp_incr));
        else
            msg.CorrectTimeStamp(ServerTime());
        SendPacket(m_socket, msg.Packet());
    }

    void Client::HandleIncomingMoveState(ByteBuffer& data)
    {
        ClientMoveState state;
        data >> state;

        state.ms_time = ClientToServerTime(state.ms_time).time;    // convert client ticks to server ticks

        if (m_controlled)
            m_controlled->m_moveEvents.QueueState(state);
    }

    void Client::CleanReferences()
    {
        if (m_controlled && m_controlled->client == this)
        {
            m_controlled->client = NULL;
        }
        m_controlled = NULL;
        m_local = NULL;
    }

    void Client::Dereference(const UnitMovement * m)
    {
        if (m != m_controlled || m_controlled->client != this)
        {
            log_write("wtf?");
            return;
        }

        if (m == m_local)
            m_local = NULL;

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
            SendPacket(m_socket, data);
        }
    }

    void Client::HandleMoveTimeSkipped(WorldPacket & recv_data)
    {
        ObjectGuid guid;
        int32 time;
        recv_data >> guid.ReadAsPacked();
        recv_data >> time;
        time_skipped = time;
    }
}
