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
            if (m_client->controlled())
                new TimeSyncRequest(m_client);
            RescheduleTaskWithDelay(args, TimeSyncRequest::SyncTimePeriod);
        }

        /** If enabled, takes into account latency. */
        static volatile bool TimeCorrectionEnabled;

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
                m_requestSendTime = Imports.getMSTime();
                Packet data(SMSG_TIME_SYNC_REQ, 4);
                data << requestId();
                client->SendPacket(data);
            }

            bool OnReply(ClientImpl * client, Packet& data) override
            {
                uint32 client_req_id;
                MSTime client_ticks;
                data >> client_req_id;
                data >> client_ticks;
                if (!checkRequestId(client_req_id))
                    return false;
                MSTime latency = TimeCorrectionEnabled ? ((ClientImpl::ServerTime() - m_requestSendTime).time / 2) : 0;
                client->SetClientTime(client_ticks + latency);
                client->latency = latency;
                return true;
            }
        };
    };

    volatile bool TimeSyncRequestScheduler::TimeCorrectionEnabled = true;

    struct SyncTimeCorrectToggleCommand : public MovementCommand
    {
        SyncTimeCorrectToggleCommand() {
            Init("sync");
            Description = "Toggles time correction option that helps determine client-side time more precisely.";
        }

        void Invoke(StringReader& /*command*/, CommandInvoker& invoker) override {
            bool on = TimeSyncRequestScheduler::TimeCorrectionEnabled ^= true;
            invoker.output << endl << "Time correction " << (on ? "enabled" : "disabled");
        }
    };
    DELAYED_INIT(SyncTimeCorrectToggleCommand,SyncTimeCorrectToggleCommand);
    struct SetClientTimeMod : public MovementCommand
    {
        explicit SetClientTimeMod() {
            Init("TimeMod");
            Description = "Modifies movement packet time deltas.";
        }

        void Invoke(StringReader& command, CommandInvoker& invoker) override {
            if (ClientImpl * cl = invoker.com.as<UnitMovement>().Impl().client()) {
                cl->timeMod = command.readInt();
                invoker.output << endl << "time mod is " << cl->timeMod;
            }
            else
                invoker.output << endl << "Unit is not client controlled";
        }
    };
    DELAYED_INIT(SetClientTimeMod, SetClientTimeMod);

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
        m_socket = nullptr;
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

    void ClientImpl::ToString(QTextStream& str) const
    {
        str << endl << "Client info";
        str << endl << "skipped time " << timeSkipped.time;
        str << endl << "latency      " << latency.time;
        str << endl << "time mod     " << timeMod;
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
                Unit_Passenger::dealloc( unit.getAspect<Unit_Passenger>() );

                if (newFlags.ontransport && state.transport_guid.GetRawValue() != 0) {
                    MovingEntity_WOW * transp = unit.context->registry.get<MovingEntity_WOW>(state.transport_guid);
                    assert_state( transp );
                    Unit_Passenger::create(unit, *transp, nullptr, state.transport_seat);
                }
            }

            unit.RelativeLocation(unit.Environment() ? state.relativePosition : state.globalPosition);
            unit.PitchAngle(state.pitchAngle);
            unit.SetMoveFlag(newFlags);
            unit.m_unused = state;
            unit.lastMoveEvent = state.ms_time;
            if (state.floatValueType != Parameter_End)
                unit.SetParameter(state.floatValueType, state.floatValue);
        }
    };

    void ClientImpl::QueueState(ClientMoveStateChange& client_state, const ObjectGuid& source)
    {
        assertControlled();
        assert_state(source == controlled()->Guid);

        //client_state.ms_time = ClientToServerTime(client_state.ms_time); // convert client to server time

        if (!controlled()->moveFlags.hasFlag(UnitMoveFlag::Mask_Moving)
            && client_state.moveFlags.hasFlag(UnitMoveFlag::Mask_Moving))
        {
            // movement has began, need reset timers
            timeSkipped = 0;
            timeLine = ClientToServerTime(client_state.ms_time);
            lastClientStamp = client_state.ms_time;
        }

        MSTime timeDiff = client_state.ms_time - lastClientStamp;

        // Disabled because not required: client increases timestamps by self
        //timeDiff += timeSkipped;
        //timeSkipped = 0;

        timeLine += timeDiff;
        timeLine += timeMod; // for test purposes only

        //MSTime deltaS = ServerTime() - lastStamp;
        //MSTime deltaC = client_state.ms_time - lastStamp;
        //log_debug("deltaS %d", deltaS.time);
        //log_debug("deltaC %d", deltaC.time);
        log_debug("timeDiff: %u", (client_state.ms_time - lastClientStamp).time);
        log_debug(qPrintable(client_state.toString()));

        lastClientStamp = client_state.ms_time;
        client_state.ms_time = timeLine;
        m_controlled->commonTasks.AddTask(new ApplyStateTask(*m_controlled,client_state), timeLine);
    }

    void ClientImpl::OnMoveTimeSkipped(ClientImpl& client, Packet & recv_data)
    {
        client.assertControlled();

        ObjectGuid guid;
        int32 skipped;
        recv_data >> guid.ReadAsPacked();
        recv_data >> skipped;

        client.timeSkipped += skipped;
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

        log_debug("OnSplineDone: arrive time desync is: %d", (state.ms_time - move_spline.ArriveTime()).time);
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

        if (UnitMovement * movem = client.m_context.registry.get<UnitMovement>(guid)) {
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

    void UnitMovementImpl::CleanReferences() {
        if (m_client) {
            m_client->Dereference(this);
            m_client = nullptr;
        }
        commonTasks.Unregister();
        MovingEntity_WOW::CleanReferences();
    }
    
    void UnitMovementImpl::toString(QTextStream& st) const {
        MovingEntity_WOW::toString(st);

        if (m_client)
            m_client->ToString(st);
    }

    //////////////////////////////////////////////////////////////////////////

    struct ClientMemoryLayout : public Client {
        explicit ClientMemoryLayout(void* socket, Context &context) : impl(socket, context) {}
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

    Client* Client::create(void * socket, Context &context)
    {
        return new ClientMemoryLayout(socket, context);
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
