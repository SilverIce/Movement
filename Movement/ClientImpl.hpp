#pragma once

namespace Movement
{
    /** Sends to CMSG_TIME_SYNC_RESP each 10 seconds */
    struct TimeSyncRequestScheduler : Executor<TimeSyncRequestScheduler,true>
    {
        ClientImpl * m_client;

        TimeSyncRequestScheduler(ClientImpl * client) : m_client(client) {
            client->commonTasks.AddTask(this, MSTime(0));
        }

        void Execute(TaskExecutor_Args& args){
            new TimeSyncRequest(m_client);
            args.executor.AddTask(args.callback, args.now + TimeSyncRequest::SyncTimePeriod, args.objectId);
        }

    private:
        class TimeSyncRequest : public RespHandler
        {
            MSTime m_requestSendTime;
        public:
            enum{
                SyncTimePeriod = 10000, // 10 seconds
            };

            TimeSyncRequest(ClientImpl * client) : RespHandler(CMSG_TIME_SYNC_RESP, client)
            {     
                WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
                data << m_reqId;
                client->SendPacket(data);
                m_requestSendTime = MaNGOS_API::getMSTime();
            }

            virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
            {
                uint32 client_req_id;
                MSTime client_ticks;
                data >> client_req_id;
                data >> client_ticks;
                if (!checkRequestId(client_req_id))
                    return false;
                MSTime latency = (MaNGOS_API::getMSTime() - m_requestSendTime.time) / 2;
                client->SetClientTime(client_ticks + latency);
                return true;
            }
        };
    };

    void ClientImpl::HandleOutcomingMessage(WorldPacket& recv_data)
    {
        if (!m_controlled){
            log_function("no controlled object");
            return;
        }

        ObjectGuid guid;
        ClientMoveStateChange state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state.state;

        QueueState(state);

        MovementMessage msg(m_controlled, recv_data.GetOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state.state;
        BroadcastMessage(msg);
    }

    /* For test. if set to true, enables old & wrong time correction */
    static bool send_self = false;

    static MSTime timestamp_incr = 5000;
    static MSTime timestamp_decr = 0;

    void ClientImpl::SendMoveMessage(MovementMessage& msg) const
    {
        if (msg.Source() == m_controlled && !send_self)
            return;

        msg.CorrectTimeStamp(msg.OrigTime() + timestamp_incr - timestamp_decr);
        SendPacket(msg.Packet());
    }

    void ClientImpl::CleanReferences()
    {
        m_resp_handlers.clear();
        LostControl();
        commonTasks.Unregister();
        m_socket = NULL;
    }

    ClientImpl::~ClientImpl()
    {
        CleanReferences();

        mov_assert(m_socket == NULL);
        mov_assert(m_controlled == NULL);
        mov_assert(m_resp_handlers.empty());
    }

    void ClientImpl::Dereference(const UnitMovementImpl * m)
    {
        if (m != m_controlled || m_controlled->client() != this)
        {
            log_function("wtf?");
            return;
        }

        LostControl();
    }

    void ClientImpl::SetControl(UnitMovementImpl * newly_controlled)
    {
        if (!commonTasks.hasExecutor())
        {
            commonTasks.SetExecutor(newly_controlled->Updater());
            new TimeSyncRequestScheduler(this);
        }
        
        LostControl();
        m_controlled = newly_controlled;
        m_controlled->client(this);
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

    void ClientImpl::HandleMoveTimeSkipped(WorldPacket & recv_data)
    {
        if (!m_controlled){
            log_function("no controlled object");
            return;
        }

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
        mov_assert(m_controlled); // wrong state

        if (m_resp_handlers.empty())
        {
            log_function("no handlers for client's response (opcode %s)", LookupOpcodeName(data.GetOpcode()));
            return;
        }

        if (m_resp_handlers.front()->OnReply(data))
            m_resp_handlers.erase(m_resp_handlers.begin());
        else
            log_function("client's response (opcode %s) can not be handled", LookupOpcodeName(data.GetOpcode()));
    }

    void ClientImpl::RegisterRespHandler(RespHandler* handler)
    {
        handler->m_reqId = request_counter.NewId();
        m_resp_handlers.push_back(handler);
        commonTasks.AddTask(handler, ServerTime() + RespHandler::DefaultTimeout);
    }

    void ClientImpl::UnregisterRespHandler(RespHandler* handler)
    {
        m_resp_handlers.remove(handler);
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
        char * data = (char*)operator new(sizeof(Client) + sizeof(ClientImpl));
        ClientImpl * impl = new(data+sizeof(Client))ClientImpl(socket);
        Client * client = new(data)Client(*impl);
        return client;
    }

    Client::~Client()
    {
        m.~ClientImpl();
    }
}
