namespace Movement
{
    using Tasks::TaskTarget;
    using Tasks::TaskExecutor_Args;
    using Tasks::CallBack;
    using Tasks::CallBackPublic;
    using Tasks::StaticExecutor;
    using Tasks::Executor;
    using Tasks::TaskTarget_DEV;

    class UnitMovementImpl;

    class MoveSplineUpdatable : public Executor<MoveSplineUpdatable,false>
    {
    private:
        MSTime m_lastQuery;
        MoveSpline m_base;
        UnitMovementImpl& m_owner;
        IListener * m_listener;
        bool m_moving;
        TaskTarget_DEV m_task;

        std::vector<OnEventArgs> events;

    private:

        enum{
        /** Spline movement update frequency, milliseconds */
            UpdateDelay = 400,
        };

        inline MSTime NextSegmentTime() const {
            return m_lastQuery + std::min(m_base.next_timestamp() - m_base.timePassed(), (int32)UpdateDelay);
        }

        void recache(int32 recacheDelay = 100);

        void PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new);
    public:
        void Execute(TaskExecutor_Args& args);
    private:

        void Disable();

    public:
        bool isEnabled() const { return m_moving;}
        void SetListener(IListener * listener) { m_listener = listener;}
        void ResetLisener() { m_listener = NULL; }

        void Launch(MoveSplineInitArgs& args);
        std::string ToString() const { return m_base.ToString();}

        explicit MoveSplineUpdatable(UnitMovementImpl& owner);

        ~MoveSplineUpdatable() {
            Disable();
            m_task.Unregister();
            ResetLisener();
        }

        const MoveSpline& moveSpline() const { return m_base;}

        const Vector3& destination() const;

        uint32 getId() const
        {
            if (isEnabled())
                return m_base.GetId();
            else
                return 0;
        }

        int32 timeElapsed() const
        {
            if (isEnabled())
                return m_base.timeElapsed();
            return 0;
        }

        int32 timePassed() const
        {
            if (isEnabled())
                return m_base.timePassed();
            return 0;
        }
    };

    class MoveSplineUpdatablePtr
    {
        MoveSplineUpdatable m_base;
    public:
        explicit MoveSplineUpdatablePtr(UnitMovementImpl& owner) : m_base(owner) {}

        ~MoveSplineUpdatablePtr() {
        }

        inline MoveSplineUpdatable* operator ->() {
            return &m_base;
        }
        inline const MoveSplineUpdatable* operator ->() const {
            return &m_base;
        }
    };
}
