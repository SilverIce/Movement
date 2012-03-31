namespace Movement
{
    class VehicleImpl : public Component, public OnPassengerDestroy
    {
        COMPONENT_TYPEID;
        enum{
            SeatCount = 8,
        };

        Unit_Passenger * m_passengers[SeatCount];
        uint32 m_vehicleId;

    public:

        uint32 VehicleId() const {
            return m_vehicleId;
        }

        explicit VehicleImpl(UnitMovementImpl& transportOwner, uint32 vehicleId)
        {
            memset(m_passengers, 0, sizeof m_passengers);
            m_vehicleId = vehicleId;

            transportOwner.ComponentAttach(this);

            WorldPacket data(SMSG_SET_VEHICLE_REC_ID, 16);
            data << transportOwner.Guid.WriteAsPacked();
            data << (uint32)m_vehicleId;
            Imports.BroadcastMessage(transportOwner.Owner, data);
        }

        ~VehicleImpl() {
            CleanReferences();
            for (int seat = 0; seat < SeatCount; ++seat)
                assert_state(!m_passengers[seat]);
        }

        void CleanReferences() {
            UnboardAll();
            ComponentDetach();
        }

        void UnboardAll() {
            for (int seat = 0; seat < SeatCount; ++seat)
                UnBoard(seat);
        }

    #pragma region  event handlers
    private:

        void onDestroy(Unit_Passenger & psg) override {
            assert_state(m_passengers[psg.SeatId()] == &psg);
            m_passengers[psg.SeatId()] = nullptr;
            OnUnboarded(psg);
        }

        void Onboarded(Unit_Passenger & psg)
        {
            // put your custom code here
            Location seatOffset(Vector3(), 0);
            MoveSplineInit init(psg.ToUnit());
            init.MoveTo(seatOffset);
            init.SetFacing(seatOffset.orientation);
            init.Launch();
            //ModeChangeEffect::Launch(&psg.ToUnit(), MoveModeRoot, true);
            //TeleportEffect::Launch(&ToUnit(), seatOffset);
        }

        void OnUnboarded(Unit_Passenger & psg)
        {
            // put your custom code here
            MoveSplineInit init(psg.ToUnit());
            //init.SetInstant();
            init.Launch();
            //ModeChangeEffect::Launch(&psg.ToUnit(), MoveModeRoot, false);
        }
    #pragma endregion

    public:

        void Board(uint8 seatId, UnitMovementImpl& passenger)
        {
            mov_assert(seatId < SeatCount);

            if (passenger.Tree() == Tree()) {
                log_function("attempt to board self");
                return;
            }

            Unit_Passenger::dealloc(passenger.getAspect<Unit_Passenger>());
            Unit_Passenger::dealloc(m_passengers[seatId]);
            m_passengers[seatId] = Unit_Passenger::create(passenger, *getAspect<MovingEntity_WOW>(), this, seatId);
            Onboarded(*m_passengers[seatId]);
        }

        void UnBoard(uint8 seatId) {
            mov_assert(seatId < SeatCount);
            Unit_Passenger::dealloc(m_passengers[seatId]);
        }

        UnitMovementImpl* getPassenger(uint8 seatId) const {
            mov_assert(seatId < SeatCount);
            if (Unit_Passenger * psg = m_passengers[seatId])
                return &psg->ToUnit();
            else
                return nullptr;
        }
    };

    void registerVehicleHandlers()
    {
        ClientOpcode opcodes[] = {
            CMSG_SET_VEHICLE_REC_ID_ACK,
            CMSG_DISMISS_CONTROLLED_VEHICLE,
            CMSG_REQUEST_VEHICLE_EXIT,
            CMSG_REQUEST_VEHICLE_PREV_SEAT,
            CMSG_REQUEST_VEHICLE_NEXT_SEAT,
            CMSG_REQUEST_VEHICLE_SWITCH_SEAT,
            CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE,
            CMSG_RIDE_VEHICLE_INTERACT,
        };
        HandlersHolder::assignHandler(&ClientImpl::OnNotImplementedMessage, opcodes, CountOf(opcodes));
    }
    DELAYED_CALL(registerVehicleHandlers);

    int8 UnitPassenger::SeatId() {
        return m->SeatId();
    }

    void Vehicle::Install(UnitMovement& transportOwner, uint32 vehicleId) {
        new VehicleImpl(transportOwner.Impl(), vehicleId);
    }

    void Vehicle::Board(int8 seatId, UnitMovement& passenger) {
        m->Board(seatId, passenger.Impl());
    }

    UnitMovement* Vehicle::Passenger(int8 seatId) {
        if (UnitMovementImpl* unit = m->getPassenger(seatId))
            return unit->PublicFace;
        else
            return nullptr;
    }

    void Vehicle::UnBoard(int8 seatId) {
        return m->UnBoard(seatId);
    }

    void Vehicle::UnboardAll() {
        m->UnboardAll();
    }

    uint32 Vehicle::VehicleId() {
        return m->VehicleId();
    }
}
