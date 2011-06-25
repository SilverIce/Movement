#include "ClientMovement.h"
#include "UnitMovement.h"
#include "ObjectGuid.h"
#include "MovementMessage.h"

#include <sstream>

namespace Movement
{
    class TimeSyncRequest : public RespHandler
    {
        uint32 reqId;
    public:

        TimeSyncRequest(Client * client) : RespHandler(CMSG_TIME_SYNC_RESP), reqId(client->AddRespHandler(this))
        {     
            WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
            data << reqId;
            client->SendPacket(data);
        }

        virtual void OnReply(Client * client, WorldPacket& data) override
        {
            uint32 client_req_id;
            MSTime client_ticks;
            data >> client_req_id;
            data >> client_ticks;
            if (client_req_id != reqId)
            {
                log_write("TimeSyncRequest::OnReply: wrong counter value: %u and should be: %u", client_req_id, reqId);
                return;
            }
            client->SetClientTime(client_ticks);
        }
    };

    void Client::HandleOutcomingMessage(WorldPacket& recv_data)
    {
        if (!m_controlled)
            return;
        ObjectGuid guid;
        ClientMoveState state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state;

        QueueState(state);

        MovementMessage msg(m_controlled, recv_data.GetOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state;
        BroadcastMessage(msg);
    }

    /* For test. if set to true, enables old & wrong time correction */
    static bool old_way = false;
    static bool send_self = false;

    static MSTime timestamp_incr = 5000;
    static MSTime timestamp_decr = 0;

    void Client::SendMoveMessage(MovementMessage& msg) const
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
        while(!m_resp_handlers.empty())
        {
            delete m_resp_handlers.back();
            m_resp_handlers.pop_back();
        }

        LostControl();
        m_socket = NULL;
    }

    void Client::Dereference(const UnitMovement * m)
    {
        if (m != m_controlled || m_controlled->client() != this)
        {
            log_write("wtf?");
            return;
        }

        LostControl();
    }

    void Client::SetControl(UnitMovement * newly_controlled)
    {
        LostControl();
        m_controlled = newly_controlled;
        m_controlled->client(this);

        new TimeSyncRequest(this);
    }

    void Client::LostControl()
    {
        if (m_controlled && m_controlled->client() == this)
            m_controlled->client(NULL);
        m_controlled = NULL;
    }

    Client::Client(HANDLE socket) :
        m_socket(socket),
        m_controlled(NULL)
    {
    }

    void Client::_OnUpdate()
    {
        MSTime now = ServerTime();

        if ((now - m_last_sync_time).time > 10000)
        {
            m_last_sync_time = now;
            new TimeSyncRequest(this);
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
        str << "Request  counter: " << request_counter.getCurrent() << std::endl;
        return str.str();
    }

    void Client::HandleResponse(WorldPacket& data)
    {
        if (!m_controlled)
            return;

        struct _handler
        {
            Client * client;
            WorldPacket& data;

            _handler(Client * c, WorldPacket& _data) : client(c), data(_data) {}

            inline bool operator()(RespHandler* hdl)
            {
                if (!hdl->CanHandle(data.GetOpcode()))
                    return false;
                hdl->OnReply(client, data);
                delete hdl;
                return true;
            }
        };

        m_resp_handlers.erase(std::find_if(m_resp_handlers.begin(),m_resp_handlers.end(),_handler(this,data)));
    }
}
