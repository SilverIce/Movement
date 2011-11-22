namespace Movement
{
    MoveSplineUpdatable::MoveSplineUpdatable(UnitMovementImpl& owner) : m_owner(owner),
        m_listener(NULL), m_moving(false)
    {
    }

    void MoveSplineUpdatable::Disable()
    {
        m_moving = false;
        m_owner.ApplyMoveFlag(UnitMoveFlag::Spline_Enabled|UnitMoveFlag::Mask_Directions|UnitMoveFlag::Mask_Moving, false);
    }

    const Vector3& MoveSplineUpdatable::destination() const
    {
        if (isEnabled())
            return m_base.FinalDestination();
        else
            return m_owner.GetPosition3();
    }

    void MoveSplineUpdatable::recache(int32 recacheDelay)
    {
        MSTime timeNow = MaNGOS_API::getMSTime();
        int32 difftime = (timeNow - m_lastQuery).time;
        if (difftime < recacheDelay || !isEnabled())
            return;

        m_lastQuery = timeNow;

        struct ResHandler
        {
            MoveSplineUpdatable& m_owner;
            std::vector<OnEventArgs>& events;
            bool needSync;

            void operator()(MoveSpline::UpdateResult result)
            {
                const MoveSpline& moveSpline = m_owner.moveSpline();
                switch (result) {
                case MoveSpline::Result_NextSegment:
                    events.push_back( OnEventArgs::OnPoint(moveSpline.GetId(),moveSpline.currentPathIdx()) );
                    break;
                case MoveSpline::Result_Arrived:
                    m_owner.Disable();
                    events.push_back( OnEventArgs::OnPoint(moveSpline.GetId(),moveSpline.currentPathIdx()+1) );
                    events.push_back( OnEventArgs::OnArrived(moveSpline.GetId()) );
                    break;
                case MoveSpline::Result_NextCycle:
                    needSync = true;
                    break;
                }
            }
        } hdl = {*this, events, false};

        m_base.updateState(difftime,hdl);
        m_owner.SetPosition(m_base.ComputePosition());

        if (hdl.needSync)
            PacketBuilder::SplineSyncSend(m_owner,MsgBroadcast(m_owner));
    }

    void MoveSplineUpdatable::Execute(TaskExecutor_Args& args)
    {
        if (isEnabled()) {
            recache(1);
            if (isEnabled())    // migh became disabled during recache
                args.executor.AddTask(args.callback, NextUpdateTime(), args.objectId);
        }
        if (uint32 size = events.size()) {
            uint32 itr = 0;
            while (m_listener && itr < size)
                m_listener->OnEvent(events[itr++]);
            events.clear();
        }
    }

    void MoveSplineUpdatable::Launch(MoveSplineInitArgs& args)
    {
        UnitMoveFlag moveFlag_new = m_owner.moveFlags;
        PrepareMoveSplineArgs(args, moveFlag_new);
        if (!args.Validate())
            return;

        m_base.Initialize(args);
        m_lastQuery = MaNGOS_API::getMSTime();
        m_moving = true;

        if (!m_task.hasExecutor())
            m_task.SetExecutor(m_owner.Updater());
        m_task.CancelTasks();
        m_task.AddTask(new ITaskP0<MoveSplineUpdatable>(this,&MoveSplineUpdatable::Execute), NextUpdateTime());

        m_owner.SetMoveFlag(moveFlag_new);
        m_owner.SetParameter(Parameter_SpeedMoveSpline, args.velocity);

        PacketBuilder::SplinePathSend(m_owner, MsgBroadcast(m_owner));
    }

    void MoveSplineUpdatable::PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new)
    {
        // There is a big chance that current position is outdated in case movement was already launched.
        // So, to lauch new movement from current _real_ position need update old state
        if (isEnabled())
            recache(1);

        mov_assert(!args.path.empty());
        args.path[0] = m_owner.GetPosition3();    //correct first vertex
        args.splineId = m_owner.Updater().NewMoveSplineId();
        args.initialOrientation = m_owner.GetPosition().orientation;

        // logic from client here:
        {
            moveFlag_new &= ~(UnitMoveFlag::Mask_Directions | UnitMoveFlag::Mask_Moving);
            moveFlag_new.backward = args.flags.backward;
            //moveFlag_new.forward = !args.flags.backward && !args.flags.falling;
            moveFlag_new.forward = true;
            moveFlag_new.spline_enabled = true;
            moveFlag_new.walk_mode = !args.flags.runmode;

            if (args.flags.falling) // disable flying
            {
                moveFlag_new &= ~(UnitMoveFlag::Pitch_Up | UnitMoveFlag::Pitch_Down | UnitMoveFlag::Ascending |
                    UnitMoveFlag::Descending | UnitMoveFlag::Flying);
            }
        }

        // select velocity if was not selected
        if (args.velocity == 0.f)
            args.velocity = m_owner.GetParameter(UnitMovementImpl::SelectSpeedType(moveFlag_new & ~UnitMoveFlag::Spline_Enabled));
    }
}
