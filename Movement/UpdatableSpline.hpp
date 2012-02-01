namespace Movement
{
    void MoveSplineUpdatable::Init(Component& tree) 
    {
        tree.ComponentAttach(this);
        m_owner = getAspect<UnitMovementImpl>();
        assert_state(m_owner);
        m_updater = &m_owner->Updater();
    }

    void MoveSplineUpdatable::OnArrived()
    {
        m_moving = false;
        m_owner->ApplyMoveFlag(UnitMoveFlag::Spline_Enabled, false);
    }

    void MoveSplineUpdatable::updateState(int32 recacheDelay)
    {
        if (!IsMoving())
            return;

        MSTime timeNow = Imports.getMSTime();
        int32 difftime = (timeNow - m_lastQuery).time;
        if (difftime < recacheDelay)
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
                    events.push_back( OnEventArgs::OnPoint(moveSpline.GetId(),moveSpline.currentPathPointIdx()) );
                    break;
                case MoveSpline::Result_Arrived:
                    m_owner.OnArrived();
                    events.push_back( OnEventArgs::OnPoint(moveSpline.GetId(),moveSpline.currentPathPointIdx()) );
                    events.push_back( OnEventArgs::OnArrived(moveSpline.GetId()) );
                    break;
                case MoveSpline::Result_NextCycle:
                    needSync = true;
                    break;
                }
            }
        } hdl = {*this, events, false};

        m_base.updateState(difftime,hdl);
        m_owner->SetRelativePosition(m_base.ComputePosition());

        if (hdl.needSync)
            PacketBuilder::SplineSyncSend(controlled());
    }

    MSTime MoveSplineUpdatable::NextUpdateTime() const
    {
        assert_state(IsMoving());

        // Compute a such next movement update time
        // that controlled movement will reach 'distance' (~3.5 yards) after that time passed
        // for ex: if current movement speed is 2.5 then delay between updates is 3.5/2.5 = 1.4 seconds
        // The main goal is performance: need update it not too frequently
        const float distance = 3.5f;
        int32 moveTime = int32(distance / m_owner->GetCurrentSpeed() * 1000.f);
        return m_lastQuery + std::min<int32>(m_base.timeElapsed(), moveTime);
    }

    void MoveSplineUpdatable::OnUpdateCallback(TaskExecutor_Args& args)
    {
        updateState(1);
        if (IsMoving())    // ensure that moving, it migh became stopped after state update
            args.executor.AddTask(args.callback, NextUpdateTime(), args.objectId);
        if (uint32 size = events.size()) {
            uint32 itr = 0;
            while (m_listener && itr < size)
                m_listener->OnEvent(events[itr++]);
            events.clear();
        }
    }

    void MoveSplineUpdatable::Launch(MoveSplineInitArgs& args)
    {
        // can't move while rooted
        if (m_owner->HasMode(MoveModeRoot))
            return;

        UnitMoveFlag moveFlag_new;
        PrepareMoveSplineArgs(args, moveFlag_new);
        if (!args.Validate())
            return;

        m_base.Initialize(args);
        m_lastQuery = Imports.getMSTime();
        m_moving = true;

        m_updater->CancelTasks(m_updateMovementTask);
        m_updater->AddTask(NewITaskP0(this,&MoveSplineUpdatable::OnUpdateCallback), NextUpdateTime(), m_updateMovementTask);

        m_owner->SetMoveFlag(moveFlag_new);
        m_owner->SetParameter(Parameter_SpeedMoveSpline, args.velocity);

        PacketBuilder::SplinePathSend(controlled());
    }

    void MoveSplineUpdatable::PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new)
    {
        if (IsMoving())
            updateState(1);

        mov_assert(!args.path.empty());
        args.path[0] = m_owner->GetRelativePosition();    //correct first vertex
        args.splineId = m_updater->NewMoveSplineId();
        args.initialOrientation = m_owner->GetOrientation();

        moveFlag_new = m_owner->moveFlags;
        // logic from client here:
        {
            moveFlag_new &= ~(UnitMoveFlag::Mask_Directions | UnitMoveFlag::Mask_Moving);
            //moveFlag_new.forward = true;
            moveFlag_new.spline_enabled = true;
            moveFlag_new.walk_mode = !args.flags.runmode;

            if (args.flags.falling) // disable flying
            {
                moveFlag_new &= ~(UnitMoveFlag::Pitch_Up | UnitMoveFlag::Pitch_Down | UnitMoveFlag::Ascending |
                    UnitMoveFlag::Descending | UnitMoveFlag::Flying);
            }

            if (!args.flags.isFacing() && IsOrientationBinded())
            {
                args.facing.target = m_targetGuid.GetRawValue();
                args.flags.EnableFacingTarget();
            }
        }

        // select velocity if was not selected
        if (args.velocity == 0.f)
            args.velocity = m_owner->GetParameter(UnitMovementImpl::SelectSpeedType(moveFlag_new & ~UnitMoveFlag::Spline_Enabled));
    }

    void MoveSplineUpdatable::BindOrientationTo(const UnitMovementImpl& target)
    {
        if (&target == m_owner)
        {
            log_function("trying to target self, skipped");
            return;
        }

        struct OrientationUpdater : public ICallBack
        {
            MoveSplineUpdatable& me;
            MSTime lastOrientationUpdate;

            enum{
                RotationUpdateDelay = 250,
            };

            explicit OrientationUpdater(MoveSplineUpdatable& _me) : me(_me) {}

            void Execute(TaskExecutor_Args& args) override
            {
                assert_state(me.IsOrientationBinded());

                UnitMovementImpl& controlled = me.controlled();
                if (UnitMovement * target = Imports.GetUnit2(controlled.Owner, me.m_targetGuid.GetRawValue()))
                {
                    args.executor.AddTask(args.callback,args.now + RotationUpdateDelay,args.objectId);
                    lastOrientationUpdate = args.now;
                    if (controlled.IsMoving() || controlled.IsClientControlled() ||   // task inactive while moving or in controlled by client state
                        // or target's position wasn't changed since last orientation update
                        (target->Impl().lastPositionChangeTime < lastOrientationUpdate))
                        return;

                    const Vector3& targetPos = target->GetPosition3();
                    const Vector3& myPos = controlled.GetRelativePosition();
                    float orientation = atan2f(targetPos.y - myPos.y, targetPos.x - myPos.x);
                    controlled.SetOrientation(orientation);
                } 
            }
        };

        // create OrientationUpdater task only in case there is no more such tasks
        if (!m_updateRotationTask.isRegistered())
            m_updater->AddTask(new OrientationUpdater(*this),0,m_updateRotationTask);
        m_targetGuid = target.Guid;
        //Owner.SetGuidValue(UNIT_FIELD_TARGET, target.Owner.GetObjectGuid());
    }

    void MoveSplineUpdatable::UnbindOrientation()
    {
        m_updater->Unregister(m_updateRotationTask);
        m_targetGuid = ObjectGuid();
        //Owner.SetGuidValue(UNIT_FIELD_TARGET, ObjectGuid());
    }
}
