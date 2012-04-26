namespace Movement
{
    using namespace Tasks;

    class UnitMovementImpl;

    Q_DECLARE_TYPEINFO(OnEventArgs, Q_PRIMITIVE_TYPE|Q_MOVABLE_TYPE);

    class MoveSplineUpdatable : public Component
    {
    private:
        COMPONENT_TYPEID(MoveSplineUpdatable);
        MSTime m_lastQuery;
        MoveSpline m_base;
        UnitMovementImpl * m_owner;
        IListener * m_listener;
        Tasks::ITaskExecutor * m_updater;
        bool m_moving;
        TaskTarget m_updateMovementTask;
        TaskTarget m_updateRotationTask;
        ObjectGuid m_targetGuid;
        QVector<OnEventArgs> events;

    private:

        enum{
        /** Spline movement update frequency, milliseconds */
            UpdateDelay = 400,
        };

        /*inline MSTime NextUpdateTime() const {
            return m_lastQuery + m_base.next_timestamp() - m_base.timePassed();
        }*/
        MSTime NextUpdateTime() const;

        void PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new);

        void OnUpdateCallback(TaskExecutor_Args& args);

        void OnArrived();

    public:
        bool IsMoving() const { return m_moving;}
        void SetListener(IListener * listener) { m_listener = listener;}
        void ResetLisener() { m_listener = NULL; }

        void Launch(MoveSplineInitArgs& args);

        void toString(QTextStream& st) const override {
            if (IsMoving()) {
                m_base.toString(st);
            }else
                st << endl << "stopped";
        }

        explicit MoveSplineUpdatable() :
            m_owner(nullptr),
            m_listener(nullptr),
            m_updater(nullptr),
            m_moving(false)
        {
        }

        void Init(UnitMovementImpl& unit);

        void CleanReferences() {
            OnArrived();
            m_updater->CancelTasks(m_updateMovementTask);
            UnbindOrientation();
            ResetLisener();
            ComponentDetach();
            m_owner = nullptr;
            m_updater = nullptr;
        }

        ~MoveSplineUpdatable() {
            assert_state(!m_listener);
            assert_state(!m_owner);
            assert_state(!m_updater);
        }

        void updateState(int32 recacheDelay = 150);

        const MoveSpline& moveSpline() const {
            assert_state(IsMoving());
            return m_base;
        }

        uint32 getCurrentMoveId() const
        {
            if (IsMoving())
                return m_base.GetId();
            else
                return 0;
        }

        uint32 getLastMoveId() const {
            return m_base.GetId();
        }

        MSTime ArriveTime() const {
            return m_lastQuery + m_base.timeElapsed();
        }

        void BindOrientationTo(const UnitMovementImpl& target);
        void UnbindOrientation();

        bool IsOrientationBinded() const {
            return m_targetGuid.GetRawValue() != 0;
        }

        UnitMovementImpl& controlled() const {
            return *m_owner;
        }
    };
}
