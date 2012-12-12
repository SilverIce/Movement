namespace Movement
{
    class Unit_Passenger;
    /** Handles notification that object-passenger will be deleted soon. */
    struct OnPassengerDestroy
    {
        virtual void onPassengerDestroy(Unit_Passenger & psg) = 0;
    };

    class Unit_Passenger : public Component
    {
        COMPONENT_TYPEID(Unit_Passenger);
        UnitMovementImpl * m_unit;
        OnPassengerDestroy * m_onDestroy;
        Tasks::TaskTarget m_updatePosTask;
        ObjectGuid m_transportGuid;
        int8 m_seatId;

        void OnUpdatePositionCallback(Tasks::TaskExecutor_Args& args) {
            RescheduleTaskWithDelay(args, 1000);
            m_unit->OnPositionChanged();
        }

        struct PassengerImpl : IPassenger
        {
            void Unboard() override {
                Unit_Passenger::dealloc(&as<Unit_Passenger>());
            }
        };

        PassengerImpl m_PassengerImpl;

    public:

        int8 SeatId() const { return m_seatId; }

        UnitMovementImpl& ToUnit() const {
            return *m_unit;
        }

        const ObjectGuid& TransportGuid() const {
            return m_transportGuid;
        }

        MovingEntity_WOW& Transport() const {
            MovingEntity_WOW * env = m_unit->Environment();
            assert_state(env);
            return *env;
        }

        void toString(QTextStream& st) const override {
            st << endl << "seat Id " << (int32)SeatId();
        }

        static Unit_Passenger* create(UnitMovementImpl& unitPassenger, MovingEntity_WOW& transport,
            OnPassengerDestroy* onDestr, int8 seatId)
        {
            return new Unit_Passenger(unitPassenger,transport,onDestr,seatId);
        }

        static void dealloc(Unit_Passenger* unitPassenger) {
            delete unitPassenger;
        }

    private:

        explicit Unit_Passenger(UnitMovementImpl& unitPassenger, MovingEntity_WOW& transport,
            OnPassengerDestroy* onDestr, int8 seatId)
        {
            m_transportGuid = transport.Guid;
            m_unit = &unitPassenger;
            m_seatId = seatId;
            m_onDestroy = onDestr;

            unitPassenger.ComponentAttach(this);
            unitPassenger.ComponentAttach<IPassenger>(&m_PassengerImpl);

            // TODO: need force stop spline movement effect before any coordinate system switch, otherwise
            // such effect will move us into wrong place
            ToUnit().SetEnvironment(&transport);
            ToUnit().ApplyMoveFlag(UnitMoveFlag::Ontransport,true);
            ToUnit().Updater().AddTask(newTask(this,&Unit_Passenger::OnUpdatePositionCallback), 0, &m_updatePosTask);
        }

        ~Unit_Passenger() {
            ToUnit().Updater().CancelTasks(m_updatePosTask);
            // TODO: need force stop spline movement effect before any coordinate system switch, otherwise
            // such effect will move us into wrong place
            ToUnit().SetEnvironment(nullptr);
            ToUnit().ApplyMoveFlag(UnitMoveFlag::Ontransport,false);

            if (m_onDestroy)
                m_onDestroy->onPassengerDestroy(*this);

            ComponentDetach();
            m_PassengerImpl.ComponentDetach();

            m_onDestroy = nullptr;
            m_unit = nullptr;
            m_onDestroy = nullptr;
            m_seatId = 0;
        }
    };
}
