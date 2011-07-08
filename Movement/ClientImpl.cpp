#pragma once

#include "Client.h"
#include "ClientImpl.h"
#include "MovementMessage.h"
#include "UnitMovement.h"

#include "ObjectGuid.h"
#include <sstream>

namespace Movement
{
    class TimeSyncRequest : public RespHandler
    {
    public:

        TimeSyncRequest(ClientImpl * client) : RespHandler(CMSG_TIME_SYNC_RESP, client)
        {     
            WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
            data << m_reqId;
            client->SendPacket(data);
        }

        virtual void OnReply(ClientImpl * client, WorldPacket& data) override
        {
            uint32 client_req_id;
            MSTime client_ticks;
            data >> client_req_id;
            data >> client_ticks;
            if (client_req_id != m_reqId)
            {
                log_write("TimeSyncRequest::OnReply: wrong counter value: %u and should be: %u", client_req_id, m_reqId);
                return;
            }
            client->SetClientTime(client_ticks);
        }
    };

    void ClientImpl::HandleOutcomingMessage(WorldPacket& recv_data)
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

    void ClientImpl::SendMoveMessage(MovementMessage& msg) const
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

    void ClientImpl::CleanReferences()
    {
        while(!m_resp_handlers.empty())
        {
            delete m_resp_handlers.back();
            m_resp_handlers.pop_back();
        }

        LostControl();
        m_socket = NULL;
    }

    void ClientImpl::Dereference(const UnitMovementImpl * m)
    {
        if (m != m_controlled || m_controlled->client() != this)
        {
            log_write("wtf?");
            return;
        }

        LostControl();
    }

    void ClientImpl::SetControl(UnitMovementImpl * newly_controlled)
    {
        LostControl();
        m_controlled = newly_controlled;
        m_controlled->client(this);

        new TimeSyncRequest(this);
    }

    void ClientImpl::LostControl()
    {
        if (m_controlled && m_controlled->client() == this)
            m_controlled->client(NULL);
        m_controlled = NULL;
    }

    ClientImpl::ClientImpl(HANDLE socket) :
        m_socket(socket),
        m_controlled(NULL)
    {
    }

    void ClientImpl::_OnUpdate()
    {
        MSTime now = ServerTime();

        if ((now - m_last_sync_time) > 10000)
        {
            m_last_sync_time = now;
            new TimeSyncRequest(this);
        }

        /*enum{response_timeout_max = 1000};
        if (!m_resp_handlers.empty() && (now - m_resp_handlers.front()->create_time).time > response_timeout_max)
        {
            // kick client here
        }*/
    }

    void ClientImpl::HandleMoveTimeSkipped(WorldPacket & recv_data)
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

    std::string ClientImpl::ToString() const
    {
        std::stringstream str;
        str << "Server-side time: " << ServerTime().time << " Client-side time: " << ClientTime().time << std::endl;
        str << "Request  counter: " << request_counter.getCurrent() << std::endl;
        return str.str();
    }

    void ClientImpl::HandleResponse(WorldPacket& data)
    {
        if (!m_controlled)
            return;

        struct _handler
        {
            ClientImpl * client;
            WorldPacket& data;

            _handler(ClientImpl * c, WorldPacket& _data) : client(c), data(_data) {}

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

    uint32 ClientImpl::AddRespHandler(RespHandler* req)
    {
        req->create_time = ServerTime();
        m_resp_handlers.push_back(req);
        return request_counter.NewId();
    }

    std::string Client::ToString() const
    {
        return m.ToString();
    }

    void Client::LostControl()
    {
        m.LostControl();
    }

    void Client::SetControl(UnitMovement * mov)
    {
        m.SetControl(&mov->Impl());
    }

    void Client::HandleResponse( WorldPacket& data )
    {
        m.HandleResponse(data);
    }

    void Client::SendMoveMessage( MovementMessage& msg ) const
    {
        m.SendMoveMessage(msg);
    }

    void Client::HandleOutcomingMessage(WorldPacket& recv_data)
    {
        m.HandleOutcomingMessage(recv_data);
    }

    void Client::HandleMoveTimeSkipped(WorldPacket & recv_data)
    {
        m.HandleMoveTimeSkipped(recv_data);
    }

    Client* Client::create(void * socket)
    {
        char* data = new char[sizeof(Client) + sizeof(ClientImpl)];
        ClientImpl * impl = new(data+sizeof(Client))ClientImpl(socket);
        Client * client = new(data)Client(*impl);
        return client;
    }

    Client::~Client()
    {
        m.~ClientImpl();
    }
}
