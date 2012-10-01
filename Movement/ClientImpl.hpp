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
            RescheduleTaskWithDelay(args, TimeSyncRequest::SyncTimePeriod);
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
                Packet data(SMSG_TIME_SYNC_REQ, 4);
                data << requestId();
                client->SendPacket(data);
                m_requestSendTime = Imports.getMSTime();
            }

            bool OnReply(ClientImpl * client, Packet& data) override
            {
                uint32 client_req_id;
                MSTime client_ticks;
                data >> client_req_id;
                data >> client_ticks;
                if (!checkRequestId(client_req_id))
                    return false;
                MSTime latency = (Imports.getMSTime() - m_requestSendTime.time) / 2;
                client->SetClientTime(client_ticks + latency);
                return true;
            }
        };
    };

    void ClientImpl::OnCommonMoveMessage(ClientImpl& client, Packet& recv_data)
    {
        client.assertControlled();

        ObjectGuid guid;
        ClientMoveStateChange state;

        recv_data >> guid.ReadAsPacked();
        recv_data >> state;

        client.QueueState(state, guid);

        MovementMessage msg(client.controlled(), recv_data.getOpcode(), recv_data.size());
        msg << guid.WriteAsPacked();
        msg << state;
        client.BroadcastMessage(msg);
    }

    void ClientImpl::SendMoveMessage(MovementMessage& msg) const
    {
        if (msg.Source() == m_controlled)
            return;

        msg.CorrectTimeStamp(msg.OrigTime());
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
            log_function("can not control a unit - client already controls some unit");
            return;
        }
        if (newly_controlled.client()) {
            log_function("can not control a unit - unit is already controlled");
            return;
        }
        if (!commonTasks.hasExecutor()) {
            // TODO: probably, would be better push executor, controlled object guid info to client constructor,
            // than pull it from controlled unit
            commonTasks.SetExecutor(newly_controlled.Updater());
            new TimeSyncRequestScheduler(this);
            m_firstControlled = newly_controlled.Guid;
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

    void ClientImpl::ToString(QTextStream& str) const
    {
        str << endl << "Server-side time: " << ServerTime().time << " Client-side time: " << ClientTime().time;
        str << endl << "Request  counter: " << m_requestCounter.getCurrent();
        if (!m_resp_handlers.empty()) {
            str << endl << "Response handlers queue:";
            foreach(const Reference<RespHandler>& hdl, m_resp_handlers)
                str << endl << typeid(*hdl.pointer()).name();
        }
    }

    uint32 ClientImpl::RegisterRespHandler(Reference<RespHandler> handler)
    {
        m_resp_handlers.push_back(handler);
        return m_requestCounter.NewId();
    }

    Reference<RespHandler> ClientImpl::PopRespHandler()
    {
        Reference<RespHandler> handler;
        if (!m_resp_handlers.empty()) {
            handler = m_resp_handlers.front();
            m_resp_handlers.pop_front();
        }
        return handler;
    }

    class ApplyStateTask : public ICallBack
    {
        UnitMovementImpl& unit;
        ClientMoveStateChange state;

        /** Only server should have permission enable or disable such flags */
        static const UnitMoveFlag::eUnitMoveFlags ImportantFlags =
            UnitMoveFlag::AllowSwimFlyTransition | UnitMoveFlag::Can_Fly | UnitMoveFlag::Hover |
            UnitMoveFlag::Waterwalking | UnitMoveFlag::GravityDisabled | UnitMoveFlag::Can_Safe_Fall | UnitMoveFlag::Root |
            UnitMoveFlag::Spline_Enabled;

        /** It tries detect not allowed state change by comparing current and new movement flags */
        bool ValidateStateChange() const
        {
            UnitMoveFlag bitChanged((unit.moveFlags.raw ^ state.moveFlags.raw) & ImportantFlags);
            if ( state.allowFlagApply == state.moveFlags.hasFlag(state.allowFlagChange) &&
                (bitChanged == state.allowFlagChange || bitChanged.raw == 0) )
            {
                return true;
            }
            else {
                log_function("invalid state change - client %s '%s' flag, but %s of '%s' flag was expected",
                    state.moveFlags.hasFlag(bitChanged) ? "enabled" : "disabled",
                    qPrintable(bitChanged.toString()),
                    state.allowFlagApply ? "enabling" : "disabling",
                    qPrintable(state.allowFlagChange.toString()));
                return false;
            }
        }

    public:

        explicit ApplyStateTask(UnitMovementImpl& own, const ClientMoveStateChange& client_state)
            : state(client_state), unit(own) {}

        void Execute(TaskExecutor_Args&) override
        {
            if (!ValidateStateChange())
                return;

            // just a bit outdated state that should not be applied
            if (unit.SplineEnabled())
                return;

            UnitMoveFlag newFlags = state.moveFlags;

            if (unit.moveFlags.ontransport != newFlags.ontransport)
            {
                if (newFlags.ontransport)
                {
                }
                else {
                }
            }

            unit.RelativeLocation(unit.Environment() ? state.relativePosition : state.globalPosition);
            unit.PitchAngle(state.pitchAngle);
            unit.SetMoveFlag(newFlags);
            unit.m_unused = state;
            if (state.floatValueType != Parameter_End)
                unit.SetParameter(state.floatValueType, state.floatValue);
        }
    };

    void ClientImpl::QueueState(ClientMoveStateChange& client_state, const ObjectGuid& source)
    {
        assertControlled();
        assert_state(source == controlled()->Guid);
        MSTime applyTime = ClientToServerTime(client_state.ms_time);
        client_state.ms_time = applyTime;

        m_controlled->commonTasks.AddTask(new ApplyStateTask(*m_controlled,client_state), client_state.ms_time);
    }

    void ClientImpl::OnMoveTimeSkipped(ClientImpl& client, Packet & recv_data)
    {
        client.assertControlled();

        ObjectGuid guid;
        int32 skipped;
        recv_data >> guid.ReadAsPacked();
        recv_data >> skipped;

        MovementMessage data(client.controlled(), MSG_MOVE_TIME_SKIPPED, 16);
        data << guid.WriteAsPacked();
        data << skipped;
        client.BroadcastMessage(data);
    }

    void ClientImpl::OnSplineDone(ClientImpl& client, Packet& data)
    {
        ObjectGuid guid;
        ClientMoveStateChange state;
        uint32 splineId;

        data >> guid.ReadAsPacked();
        data >> state;
        data >> splineId;

        client.QueueState(state, guid);

        MoveSplineUpdatable& move_spline = client.controlled()->as<MoveSplineUpdatable>();
        /*move_spline->updateState(1);
        if (splineId != move_spline->getLastMoveId())
            log_function("incorrect splineId: %u, expected %u", splineId, move_spline->getLastMoveId());*/
    }

    void ClientImpl::OnNotActiveMover(ClientImpl& client, Packet& data)
    {
        ObjectGuid guid;
        ClientMoveStateChange state;
        data >> guid.ReadAsPacked();
        data >> state;

        if (!client.controlled()){
            log_function("control already lost");
            return;
        }
        client.QueueState(state, guid);
        client.LostControl();
    }

    void ClientImpl::OnActiveMover(ClientImpl& client, Packet& data)
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

    void ClientImpl::OnNotImplementedMessage(ClientImpl&, Packet& data)
    {
        log_function("Unimplemented message handler called: %s", OpcodeName((ClientOpcode)data.getOpcode()));
    }

    //////////////////////////////////////////////////////////////////////////

    struct ClientMemoryLayout : public Client {
        explicit ClientMemoryLayout(void* socket) : impl(socket) {}
        ClientImpl impl;
    };

    void Client::SendMoveMessage(MovementMessage& msg) const
    {
        static_cast<const ClientMemoryLayout*>(this)->impl.SendMoveMessage(msg);
    }

    void Client::OnMovementMessage(const PacketData& message)
    {
        HandlersHolder::instance().InvokeHander(static_cast<ClientMemoryLayout*>(this)->impl, Packet(message));
    }

    Client* Client::create(void * socket)
    {
        return new ClientMemoryLayout(socket);
    }

    void Client::dealloc()
    {
        delete static_cast<ClientMemoryLayout*>(this);
    }

    void Client::FillSubscribeList(QVector<uint16>& opcodes)
    {
        HandlersHolder::instance().FillSubscribeList(opcodes);
    }
}
