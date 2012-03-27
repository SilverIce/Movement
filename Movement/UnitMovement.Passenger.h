namespace Movement
{
    class Unit_Passenger;
    /** Notifies transport that object-passenger will be deleted soon. */
    struct OnPassengerDestroy
    {
        virtual void onDestroy(Unit_Passenger & psg) = 0;
    };

    class Unit_Passenger : public ComponentT<Unit_Passenger>
    {
        UnitMovementImpl * m_unit;
        OnPassengerDestroy * m_onDestroy;
        Tasks::TaskTarget_DEV m_updatePosTask;
        ObjectGuid m_transportGuid;
        int8 m_seatId;

        void OnUpdatePositionCallback(Tasks::TaskExecutor_Args& args) {
            RescheduleTaskWithDelay(args, 1000);
            Imports.OnPositionChanged(m_unit->Owner, m_unit->GetGlobalLocation());
        }

        /*struct PassengerImpl : IPassenger
        {
            void Unboard() override {
                delete this->as<Unit_Passenger>();
            }
        };

        PassengerImpl m_PassengerImpl;*/

    public:

        int8 SeatId() const { return m_seatId; }

        UnitMovementImpl& ToUnit() const {
            return *m_unit;
        }

        const ObjectGuid& TransportGuid() const {
            return m_transportGuid;
        }

        explicit Unit_Passenger(UnitMovementImpl& unitPassenger, MovingEntity_WOW& transport,
            OnPassengerDestroy* onDestr, int8 seatId)
        {
            unitPassenger.ComponentAttach(this);
            //unitPassenger.ComponentAttach<IPassenger>(&m_PassengerImpl);

            m_transportGuid = transport.Guid;
            m_unit = &unitPassenger;
            m_seatId = seatId;
            m_onDestroy = onDestr;
            m_updatePosTask.SetExecutor(unitPassenger.Updater());

            // TODO: need force stop spline movement effect before any coordinate system switch, otherwise
            // such effect will move us into wrong place
            ToUnit().SetEnvironment(&transport);
            m_updatePosTask.AddTask(NewITaskP0(this,&Unit_Passenger::OnUpdatePositionCallback), 0);
        }

        ~Unit_Passenger() {
            m_updatePosTask.CancelTasks();
            // TODO: need force stop spline movement effect before any coordinate system switch, otherwise
            // such effect will move us into wrong place
            ToUnit().SetEnvironment(nullptr);
            ComponentDetach();

            if (m_onDestroy)
                m_onDestroy->onDestroy(*this);

            m_onDestroy = nullptr;
            m_unit = nullptr;
            m_onDestroy = nullptr;
            m_seatId = 0;
        }
    };
}
