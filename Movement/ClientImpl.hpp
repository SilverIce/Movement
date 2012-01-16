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
                m_requestSendTime = Imports.getMSTime();
            }

            virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
            {
                uint32 client_req_id;
                MSTime client_ticks;
                data >> client_req_id;
                data >> client_ticks;
                if (!checkRequestId(client_req_id))
                    return false;
                MSTime latency = (Imports.getMSTime() - m_requestSendTime.time) / 2;
                client->SetClientTime(client->timeRandomModifier + client_ticks + latency);
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

    static MSTime timestamp_incr = 5000;
    static MSTime timestamp_decr = 0;

    void ClientImpl::SendMoveMessage(MovementMessage& msg) const
    {
        if (msg.Source() == m_controlled)
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

    void ClientImpl::SetControl(UnitMovementImpl& newly_controlled)
    {
        if (controlled()) {
            log_function("client already controls mover");
            return;
        }
        if (newly_controlled.client()) {
            log_function("movement is already controlled");
            return;
        }
        if (!commonTasks.hasExecutor()) {
            commonTasks.SetExecutor(newly_controlled.Updater());
            new TimeSyncRequestScheduler(this);
        }
        m_controlled = &newly_controlled;
        m_controlled->client(this);
    }

    void ClientImpl::LostControl()
    {
        if (m_controlled) {
            assert_state(m_controlled->client() == this);
            m_controlled->client(NULL);
        }
        m_controlled = NULL;
    }

    ClientImpl::ClientImpl(void * socket) :
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
        if (!m_resp_handlers.empty()) {
            str << "Response handlers queue:" << std::endl;
            for (RespHdlContainer::const_iterator it = m_resp_handlers.begin();it != m_resp_handlers.end(); ++it)
                str << typeid(**it).name() << std::endl;
        }
        return str.str();
    }

    uint32 ClientImpl::RegisterRespHandler(RespHandler* handler)
    {
        m_resp_handlers.push_back(handler);
        return request_counter.NewId();
    }

    RespHandler* ClientImpl::PopRespHandler()
    {
        RespHandler* handler = NULL;
        if (!m_resp_handlers.empty()) {
            handler = m_resp_handlers.front();
            m_resp_handlers.pop_front();
        }
        return handler;
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
                log_function("client %s '%s' flag, but %s of '%s' flag was expected",
                    state.moveFlags.hasFlag(bitChanged) ? "enabled" : "disabled",
                    bitChanged.ToString().c_str(),
                    (state.allowFlagApply ? "enabling" : "disabling"),
                    state.allowFlagChange.ToString().c_str());
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

    void ClientImpl::OnSplineDone(ClientImpl& client, WorldPacket& data)
    {
        ObjectGuid guid;
        ClientMoveStateChange state;
        uint32 splineId;

        data >> guid.ReadAsPacked();
        data >> state;
        data >> splineId;

        client.QueueState(state);

        MoveSplineUpdatable * move_spline = client.controlled()->move_spline.operator->();
        if (splineId != move_spline->getLastMoveId())
            log_function("incorrect splineId: %u, expected %u", splineId, move_spline->getLastMoveId());
    }

    void ClientImpl::OnNotActiveMover(ClientImpl& client, WorldPacket& data)
    {
        ObjectGuid guid;
        ClientMoveStateChange state;
        data >> guid.ReadAsPacked();
        data >> state;

        if (!client.controlled())
            log_function("control already lost");

        client.QueueState(state);
        client.LostControl();
    }

    void ClientImpl::OnActiveMover(ClientImpl& client, WorldPacket& data)
    {
        ObjectGuid guid;
        data >> guid;

        if (UnitMovement * movem = Imports.GetUnit(client.m_socket, guid.GetRawValue())) {
            client.LostControl();
            client.SetControl(movem->Impl());
        }
        else
            log_function("can't find mover");
    }

    void ClientImpl::OnNotImplementedMessage(ClientImpl&, WorldPacket& data)
    {
        log_function("Unimplemented message handler called: %s", LookupOpcodeName((ClientOpcode)data.GetOpcode()));
    }

    //////////////////////////////////////////////////////////////////////////

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

    void Client::dealloc()
    {
        m.~ClientImpl();
        delete this;
    }

    void Client::FillSubscribeList(std::vector<uint16>& opcodes)
    {
        MoveHandlersBinder::FillSubscribeList(opcodes);
    }
}
