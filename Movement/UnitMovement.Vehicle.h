namespace Movement
{
    class VehicleImpl : public Component, public OnPassengerDestroy
    {
        COMPONENT_TYPEID;
    public:
        enum{
            SeatCount = 8,
        };
    private:
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

        std::string toString() const override {
            std::ostringstream st;
            using std::endl;
            st << endl << "vehicle Id " << m_vehicleId;
            st << endl << "passenger count " << (SeatCount - std::count(
                m_passengers,m_passengers+SeatCount,(Unit_Passenger*)0));
            return st.str();
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

            if (passenger.sameTree(passenger)) {
                log_function("attempt to board self");
                return;
            }

            Unit_Passenger::dealloc(passenger.getAspect<Unit_Passenger>());
            Unit_Passenger::dealloc(m_passengers[seatId]);
            m_passengers[seatId] = Unit_Passenger::create(passenger, as<MovingEntity_WOW>(), this, seatId);
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

    namespace VehicleHandler
    {
        static void Dismiss(ClientImpl& client, WorldPacket& data)
        {
            ObjectGuid guid;
            ClientMoveStateChange state;

            data >> guid.ReadAsPacked();
            data >> state;

            client.QueueState(state, guid);
        }

        static void ChangeSeats(ClientImpl& client, WorldPacket& data)
        {
            UnitMovementImpl& unit = client.firstControlled();
            // TODO: too much 'as' casts here..
            Unit_Passenger& psg = unit.as<Unit_Passenger>();
            VehicleImpl& veh = psg.Transport().as<VehicleImpl>();

            switch (data.GetOpcode())
            {
            case CMSG_REQUEST_VEHICLE_PREV_SEAT: {
                // TODO: vehicle have enough info about his passenger.
                // need implement a 'move' operation that moves passenger to next or previous seat
                int8 prev = (psg.SeatId() + VehicleImpl::SeatCount - 1) % VehicleImpl::SeatCount;
                veh.Board(prev, unit);
                break;
            }
            case CMSG_REQUEST_VEHICLE_NEXT_SEAT: {
                // TODO: vehicle have enough info about his passenger.
                // need implement a 'move' operation that moves passenger to next or previous seat
                int8 next = (psg.SeatId() + 1) % VehicleImpl::SeatCount;
                veh.Board(next, unit);
                break;
            }
            case CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE:
                {
                    ObjectGuid guid;        // current vehicle guid
                    ClientMoveStateChange state;
                    ObjectGuid accessory;        //  accessory guid
                    int8 seatId;
                    data >> guid.ReadAsPacked();
                    data >> state;
                    data >> accessory.ReadAsPacked();
                    data >> seatId;

                    veh.Board(seatId, unit);
                    break;
                }
            case CMSG_REQUEST_VEHICLE_SWITCH_SEAT:
                {
                    ObjectGuid guid;        // current vehicle guid
                    data >> guid.ReadAsPacked();

                    int8 seatId;
                    data >> seatId;

                    veh.Board(seatId, unit);
                    break;
                }
            default:
                assert_state(false);
                break;
            }
        }

        static void Register()
        {
            ASSIGN_HANDLER(&ChangeSeats,
                CMSG_CHANGE_SEATS_ON_CONTROLLED_VEHICLE,
                CMSG_REQUEST_VEHICLE_PREV_SEAT,
                CMSG_REQUEST_VEHICLE_NEXT_SEAT,
                CMSG_REQUEST_VEHICLE_SWITCH_SEAT);

            ASSIGN_HANDLER(&Dismiss, CMSG_DISMISS_CONTROLLED_VEHICLE);

            ASSIGN_HANDLER(&ClientImpl::OnNotImplementedMessage,
                CMSG_SET_VEHICLE_REC_ID_ACK,
                CMSG_REQUEST_VEHICLE_EXIT,
                CMSG_RIDE_VEHICLE_INTERACT);
        }
        DELAYED_CALL(Register);
    }

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
