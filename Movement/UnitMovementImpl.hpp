#pragma once

namespace Movement
{
    SpeedType UnitMovementImpl::SelectSpeedType(UnitMoveFlag moveFlags)
    {
        // g_moveFlags_mask - some global client's moveflag mask
        // TODO: get real value
        static uint32 g_moveFlags_mask = 0;
        bool use_walk_forced = false;

        //if ( !(g_moveFlags_mask & moveFlags) )
            //return 0.0f;

        if ( moveFlags.flying )
        {
            if ( moveFlags.backward /*&& speed_obj.flight >= speed_obj.flight_back*/ )
                return SpeedFlightBack;
            else
                return SpeedFlight;
        }
        else if ( moveFlags.swimming )
        {
            if ( moveFlags.backward /*&& speed_obj.swim >= speed_obj.swim_back*/ )
                return SpeedSwimBack;
            else
                return SpeedSwim;
        }
        else
        {
            if ( moveFlags.walk_mode || use_walk_forced )
            {
                //if ( speed_obj.run > speed_obj.walk )
                    return SpeedWalk;
            }
            else
            {
                if ( moveFlags.backward /*&& speed_obj.run >= speed_obj.run_back*/ )
                    return SpeedRunBack;
            }
            return SpeedRun;
        }
    }

    UnitMovementImpl::UnitMovementImpl(WorldObjectType owner) :
        MovementBase(owner),
        move_spline(*this),
        m_updater(NULL),
        m_client(NULL)
    {
        const float BaseValues[Parameter_End] =
        {
            2.5f,                                                   // SpeedWalk
            7.0f,                                                   // SpeedRun
            4.5f,                                                   // SpeedSwimBack
            4.722222f,                                              // SpeedSwim
            1.25f,                                                  // SpeedRunBack
            7.0f,                                                   // SpeedFlight
            4.5f,                                                   // SpeedFlightBack
            3.141594f,                                              // SpeedTurn
            3.141594f,                                              // SpeedPitch
            2.0f,                                                   // CollisionHeight
            7.0f,                                                   // SpeedCurrent
        };

        memcpy(m_float_values,BaseValues, sizeof m_float_values);
    }

    UnitMovementImpl::~UnitMovementImpl()
    {
        mov_assert(m_targeter_references.empty());
        mov_assert(!m_target_link.linked());
        mov_assert(m_client == NULL);
        mov_assert(m_updater == NULL);
    }

    void UnitMovementImpl::CleanReferences()
    {
        if (m_client) {
            m_client->Dereference(this);
            m_client = NULL;
        }

        struct unbinder{
            inline void operator()(TargetLink& link) { link.targeter->UnbindOrientation();}
        };
        m_targeter_references.Iterate(unbinder());

        UnbindOrientation();
        MovementBase::CleanReferences();

        commonTasks.Unregister();
        m_updater = NULL;
    }

    Vector3 UnitMovementImpl::direction() const
    {
        if (!moveFlags.hasDirection())
            return Vector3();

        float dest_angle = GetGlobalPosition().orientation;

        if (moveFlags.forward)
        {
            if (moveFlags.strafe_right)
                dest_angle -= G3D::halfPi()*0.5;
            else if (moveFlags.strafe_left)
                dest_angle += G3D::halfPi()*0.5;
        }
        else if (moveFlags.backward)
        {
            dest_angle += G3D::pi();

            if (moveFlags.strafe_right)
                dest_angle -= G3D::halfPi()*0.5;
            else if (moveFlags.strafe_left)
                dest_angle += G3D::halfPi()*0.5;
        }
        else if (moveFlags.strafe_right)
            dest_angle -= G3D::halfPi();
        else if (moveFlags.strafe_left)
            dest_angle += G3D::halfPi();

        return Vector3(cos(dest_angle), sin(dest_angle), 0);
    }

    void UnitMovementImpl::Initialize(const Location& pos, MoveUpdater& updater)
    {
        if (!m_updater)
        {
            m_updater = &updater;
            commonTasks.SetExecutor(updater);
            struct RegularUpdater : StaticExecutor<UnitMovementImpl,RegularUpdater,false> {
                static void Execute(UnitMovementImpl& me, TaskExecutor_Args& args) {
                    me.UpdateState(args.now);
                    readd(args, 200);
                }
            };
            commonTasks.AddTask(CallBackPublic(this,&RegularUpdater::Static_Execute),0);
        }

        SetPosition(pos);
        setLastUpdate(Updater().lastUpdate());
    }

    void UnitMovementImpl::ApplyState(const ClientMoveState& new_state)
    {
        if (SplineEnabled())
        {
            log_fatal("while in server control");
            return;
        }

        UnitMoveFlag new_flags = new_state.moveFlags;

        // Allow world position change only while we are not on transport
        if (!new_state.moveFlags.ontransport)
        {
            SetGlobalPosition(new_state.world_position);
        }

        if (moveFlags.ontransport != new_flags.ontransport)
        {
            if (new_flags.ontransport)
            {
                // TODO: find transport by guid, board
                // BoardOn(transport, state.transport_position, state.transport_seat);
            } 
            else
            {
                // Unboard();
            }
        }

        SetMoveFlag(new_flags);
        m_unused = new_state;
    }

    void UnitMovementImpl::BindOrientationTo(UnitMovementImpl& target)
    {
        if (&target == this)
        {
            log_function("trying to target self, skipped");
            return;
        }

        struct OrientationUpdater : StaticExecutor<UnitMovementImpl,OrientationUpdater,false>
        {
            enum{
                RotationUpdateDelay = 250,
            };
            static void Execute(UnitMovementImpl& me, TaskExecutor_Args& args) {
                mov_assert(me.IsOrientationBinded());
                args.executor.AddTask(args.callback,args.now + RotationUpdateDelay,args.objectId);
                if (me.IsMoving() || me.IsClientControlled())
                    return;

                const Vector3& targetPos = me.GetTarget()->GetGlobalPosition();
                Location myPos = me.GetPosition();
                myPos.orientation = atan2(targetPos.y - myPos.y, targetPos.x - myPos.x);
                me.SetPosition(myPos);
            }
        };

        // TODO: this place is big field for improvements. List of units that are subscibed to receive
        // orientation updates might be moved to new class. This class can be automatically deleted when none targets unit

        // create OrientationUpdater task only in case there is no more such tasks
        if (!m_updateRotationTask.isRegistered())
            m_updater->AddTask(CallBackPublic(this,&OrientationUpdater::Static_Execute),0,m_updateRotationTask);

        if (m_target_link.linked())
            m_target_link.List().delink(m_target_link);
        m_target_link.Value = TargetLink(&target, this);
        target.m_targeter_references.link_first(m_target_link);
        Owner.SetGuidValue(UNIT_FIELD_TARGET, target.Owner.GetObjectGuid());
    }

    void UnitMovementImpl::UnbindOrientation()
    {
        m_updater->Unregister(m_updateRotationTask);
        if (m_target_link.linked())
            m_target_link.List().delink(m_target_link);
        m_target_link.Value = TargetLink();
        Owner.SetGuidValue(UNIT_FIELD_TARGET, ObjectGuid());
    }

    void UnitMovementImpl::UpdateState(MSTime timeNow)
    {
    }

    std::string UnitMovementImpl::ToString() const
    {
        std::stringstream st;
        st << ClientState().ToString();
        if (m_client)
            st << m_client->ToString();
        if (SplineEnabled())
            st << move_spline->ToString();
        return st.str();
    }

    bool UnitMovementImpl::SplineEnabled() const {
        return move_spline->isEnabled();
    }

    void UnitMovementImpl::SetMoveFlag(const UnitMoveFlag& newFlags)
    {
        if ((moveFlags & UnitMoveFlag::Mask_Speed) != (newFlags & UnitMoveFlag::Mask_Speed))
        {
            SpeedType speed_type = UnitMovementImpl::SelectSpeedType(newFlags);
            m_float_values[Parameter_SpeedCurrent] = GetSpeed(speed_type);
        }
        const_cast<UnitMoveFlag&>(moveFlags) = newFlags;
    }

    ClientMoveState UnitMovementImpl::ClientState() const
    {
        ClientMoveState state;
        static_cast<_ClientMoveState>(state) = m_unused;
        state.ms_time = getLastUpdate();
        state.world_position = GetGlobalPosition();
        state.moveFlags = moveFlags;

        // correct copyed data
        state.moveFlags.ontransport = IsBoarded();
        return state;
    }
}
