#pragma once

namespace Movement
{
    /** Sends to CMSG_TIME_SYNC_RESP each 10 seconds */
    struct TimeSyncRequestScheduler : public ICallBack
    {
        ClientImpl * m_client;

        explicit TimeSyncRequestScheduler(ClientImpl * client) : m_client(client) {
            client->commonTasks.AddTask(this, MSTime(0));
        }

        void Execute(TaskExecutor_Args& args) override {
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

            explicit TimeSyncRequest(ClientImpl * client) : RespHandler(CMSG_TIME_SYNC_RESP, client)
            {
                WorldPacket data(SMSG_TIME_SYNC_REQ, 4);
                data << m_requestId;
                client->SendPacket(data);
                m_requestSendTime = Imports::getMSTime();
            }

            virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
            {
                uint32 client_req_id;
                MSTime client_ticks;
                data >> client_req_id;
                data >> client_ticks;
                if (!checkRequestId(client_req_id))
                    return false;
                MSTime latency = (Imports::getMSTime() - m_requestSendTime.time) / 2;
                client->SetClientTime(client_ticks + latency);
                return true;
            }
        };
    };

    void ClientImpl::OnCommonMoveMessage(ClientImpl& client, WorldPacket& recv_data)
    {
        client.assertControlled();

        ObjectGuid guid;
        ClientMoveStateChange state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state;

        client.QueueState(state);

        MovementMessage msg(client.controlled(), recv_data.GetOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state;
        client.BroadcastMessage(msg);
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
        assert_state(m == m_controlled && m_controlled->client() == this)
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
        assert_state(!m_controlled || (m_controlled->client() == this));
        if (m_controlled)
            m_controlled->client(NULL);
        m_controlled = NULL;
    }

    ClientImpl::ClientImpl(HANDLE socket) :
        m_socket(socket),
        m_controlled(NULL)
    {
    }

    void ClientImpl::OnMoveTimeSkipped(ClientImpl& client, WorldPacket & recv_data)
    {
        client.assertControlled();

        ObjectGuid guid;
        int32 time;
        recv_data >> guid.ReadAsPacked();
        recv_data >> time;
        //time_skipped = time;

        /*MovementMessage data(m_controlled, MSG_MOVE_TIME_SKIPPED, 16);
        data << guid.WriteAsPacked();
        data << time;
        BroadcastMessage(data);*/
    }

    std::string ClientImpl::ToString() const
    {
        std::stringstream str;
        str << "Server-side time: " << ServerTime().time << " Client-side time: " << ClientTime().time << std::endl;
        str << "Request  counter: " << request_counter.getCurrent() << std::endl;
        return str.str();
    }

    void ClientImpl::OnResponse(ClientImpl& client, WorldPacket& data)
    {
        client.assertControlled();

        RespHdlContainer& resp_handlers = client.m_resp_handlers;
        if (resp_handlers.empty())
        {
            log_function("no handlers for client's response (opcode %s)", LookupOpcodeName((ClientOpcode)data.GetOpcode()));
            return;
        }

        if (resp_handlers.front()->OnReply(data))
            resp_handlers.erase(resp_handlers.begin());
        else
            log_function("client's response (opcode %s) can not be handled", LookupOpcodeName((ClientOpcode)data.GetOpcode()));
    }

    class ApplyStateTask : public ICallBack
    {
        UnitMovementImpl * owner;
        ClientMoveStateChange state;

        /** Only server should have permission enable or disable such flags */
        static const UnitMoveFlag::eUnitMoveFlags ImportantFlags =
            UnitMoveFlag::AllowSwimFlyTransition | UnitMoveFlag::Can_Fly | UnitMoveFlag::Hover |
            UnitMoveFlag::Waterwalking | UnitMoveFlag::GravityDisabled | UnitMoveFlag::Can_Safe_Fall | UnitMoveFlag::Root |
            UnitMoveFlag::Spline_Enabled;

        /** It tries detect unallowed state change by comparing current and new movement flags */
        bool ValidateStateChange() const
        {
            UnitMoveFlag bitChanged((owner->moveFlags.raw ^ state.moveFlags.raw) & ImportantFlags);
            if ( state.allowFlagApply == state.moveFlags.hasFlag(state.allowFlagChange) &&
                (bitChanged == state.allowFlagChange || bitChanged.raw == 0) )
            {
                return true;
            }
            else {
                log_function("movement flag desync. flag difference '%s', flag difference allowed '%s'",
                    bitChanged.ToString().c_str(), state.allowFlagChange.ToString().c_str());
                return false;
            }
        }

    public:

        explicit ApplyStateTask(UnitMovementImpl * own, const ClientMoveStateChange& client_state)
            : state(client_state), owner(own) {}

        void Execute(TaskExecutor_Args&) override
        {
            if (!ValidateStateChange())
                return;

            owner->ApplyState(state);
            if (state.floatValueType != Parameter_End)
                owner->SetParameter(state.floatValueType, state.floatValue);
        }
    };

    void ClientImpl::QueueState(ClientMoveStateChange& client_state)
    {
        assertControlled();
        MSTime applyTime = ClientToServerTime(client_state.ms_time);
        client_state.ms_time = applyTime;
        m_controlled->commonTasks.AddTask(new ApplyStateTask(m_controlled,client_state), applyTime);
    }

    void ClientImpl::RegisterRespHandler(RespHandler* handler)
    {
        handler->m_requestId = request_counter.NewId();
        m_resp_handlers.push_back(handler);
        commonTasks.AddTask(handler, ServerTime() + RespHandler::DefaultTimeout);
    }

    void ClientImpl::UnregisterRespHandler(RespHandler* handler)
    {
        m_resp_handlers.remove(handler);
    }

    void ClientImpl::OnSplineDone(ClientImpl& client, WorldPacket& data)
    {
        client.assertControlled();
        ObjectGuid guid;
        ClientMoveStateChange state;
        uint32 splineId;

        data >> guid.ReadAsPacked();
        data >> state;
        data >> splineId;

        client.QueueState(state);

        if (splineId != client.controlled()->move_spline->getId())
            log_function("incorrect splineId: %u, expected %u", splineId, client.controlled()->move_spline->getId());
    }

    void ClientImpl::OnNotImplementedMessage(ClientImpl&, WorldPacket& data)
    {
        log_function("Unimplemented message handler called: %s", LookupOpcodeName((ClientOpcode)data.GetOpcode()));
    }

    //////////////////////////////////////////////////////////////////////////
    
    void Client::LostControl()
    {
        m.LostControl();
    }

    void Client::SetControl(UnitMovement * mov)
    {
        m.SetControl(&mov->Impl());
    }

    void Client::SendMoveMessage( MovementMessage& msg ) const
    {
        m.SendMoveMessage(msg);
    }

    void Client::OnMovementMessage(WorldPacket& message)
    {
        MoveHandlersBinder::InvokeHander(m, message);
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

    void Client::FillSubscribeList(std::vector<uint16>& opcodes)
    {
        MoveHandlersBinder::FillSubscribeList(opcodes);
    }
}
