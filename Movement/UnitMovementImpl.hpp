#pragma once

namespace Movement
{
    struct MsgBroadcast : public MsgDeliverer
    {
        explicit MsgBroadcast(WorldObjectType owner) : m_owner(owner) {}
        explicit MsgBroadcast(MovementBase* m) : m_owner(m->Owner) {}
        explicit MsgBroadcast(MovementBase& m) : m_owner(m.Owner) {}
        virtual void operator()(WorldPacket& data) { MaNGOS_API::BroadcastMessage(&m_owner, data);}
        WorldObjectType m_owner;
    };

    struct UniqueStateFilter
    {
        static bool Do (const ClientMoveState& prev, const ClientMoveState& next)
        {
            return true;
        }
    };

    bool MoveStateSet::Next(ClientMoveState& state, MSTime time_now)
    {
        if (m_state_queue.empty() || CurrentState().ms_time > time_now)
            return false;

        state = CurrentState();
        m_state_queue.pop_back();
        return true;
    }

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
        m_listener(NULL),
        move_spline(NULL),
        m_client(NULL)
    {
        updatable.SetUpdateStrategy(this);

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
        delete move_spline;
        move_spline = NULL;

        mov_assert(m_targeter_references.empty());
        mov_assert(m_client == NULL);
    }

    void UnitMovementImpl::CleanReferences()
    {
        if (m_client)
        {
            m_client->Dereference(this);
            m_client = NULL;
        }

        struct unbinder{
            inline void operator()(TargetLink& link) { link.targeter->UnbindOrientation();}
        };
        m_targeter_references.Iterate(unbinder());

        UnbindOrientation();
        MovementBase::CleanReferences();
        updatable.CleanReferences();
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
        SetPosition(pos);
        updatable.SetUpdater(updater);
        setLastUpdate(GetUpdater().TickTime());
    }

    void UnitMovementImpl::ApplyState(const ClientMoveState& new_state)
    {
        if (SplineEnabled())
        {
            log_write("UnitMovement::ApplyState while in server control");
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

    void UnitMovementImpl::updateRotation()
    {
        if (!IsOrientationBinded())
            return;

        const Vector3& t_pos = GetTarget()->GetGlobalPosition();
        Location my_pos = GetPosition();
        my_pos.orientation = atan2(t_pos.y - my_pos.y, t_pos.x - my_pos.x);
        SetPosition(my_pos);
        // code below calculates facing angle base on turn speed, but seems this not needed:
        // server-side conrolled unit has instant rotation speed, i.e. unit are everytime face to the target
        /*float limit_angle = G3D::wrap(atan2(t_pos.y - position.y, t_pos.x - position.x), 0.f, (float)G3D::twoPi());
        float total_angle_diff = fabs(position.w - limit_angle);
        if (total_angle_diff > 10.f/180.f * G3D::pi())
        {
            float passed_angle_diff = ms_time_diff / 1000.f * speed_obj.turn;
            passed_angle_diff = std::min(passed_angle_diff, total_angle_diff);
            position.w += passed_angle_diff;
            if (position.w > G3D::twoPi())
                position.w -= G3D::twoPi();
        }*/
    }

    void UnitMovementImpl::BindOrientationTo(UnitMovementImpl& target)
    {
        UnbindOrientation();

        if (&target == this)
        {
            log_write("UnitMovement::BindOrientationTo: trying to target self, skipped");
            return;
        }

        // can i target self?
        m_target_link.Value = TargetLink(&target, this);
        target.m_targeter_references.link(m_target_link);
        Owner.SetGuidValue(UNIT_FIELD_TARGET, target.Owner.GetObjectGuid());
    }

    void UnitMovementImpl::UnbindOrientation()
    {
        m_target_link.delink();
        m_target_link.Value = TargetLink();
        Owner.SetGuidValue(UNIT_FIELD_TARGET, ObjectGuid());
    }

    struct UnitMovementImpl::MoveSplineUpdater
    {
        UnitMovementImpl& mov;
        MoveSpline& move_spline;
        bool NeedSync;

        explicit MoveSplineUpdater(UnitMovementImpl& movement, int32 difftime) :
            mov(movement), NeedSync(false), move_spline(*mov.move_spline)
        {
            move_spline.updateState(difftime, *this);
            mov.SetPosition(move_spline.ComputePosition(mov.GetPosition()));

            if (NeedSync)
                PacketBuilder::SplineSyncSend(mov, MsgBroadcast(mov));
        }

        inline void operator()(MoveSpline::UpdateResult result)
        {
            switch (result)
            {
            case MoveSpline::Result_NextSegment:
                //log_console("UpdateState: segment %d is on hold, position: %s", move_spline.currentSplineSegment(),GetPosition3().toString().c_str());
                if (mov.m_listener)
                    mov.m_listener->OnEvent( OnEventArgs::OnPoint(move_spline.GetId(),move_spline.currentPathIdx()) );
                break;
            case MoveSpline::Result_Arrived:
                //log_console("UpdateState: spline done, position: %s", GetPosition3().toString().c_str());
                mov.DisableSpline();
                if (mov.m_listener)
                {
                    // it's never possible to have 'current point == last point', need send point+1 here
                    mov.m_listener->OnEvent( OnEventArgs::OnPoint(move_spline.GetId(),move_spline.currentPathIdx()+1) );
                    mov.m_listener->OnEvent( OnEventArgs::OnArrived(move_spline.GetId()) );
                }
                break;
            case MoveSpline::Result_NextCycle:
                NeedSync = true;
                break;
            }
        }
    };

    void UnitMovementImpl::UpdateState()
    {
        MSTime now = GetUpdater().TickTime();

        if (SplineEnabled())
        {
            int32 difftime = (now - getLastUpdate()).time;
            if (move_spline->timeElapsed() <= difftime || difftime >= MoveSpline_UpdateDelay)
            {
                MoveSplineUpdater(*this, std::min(difftime,(int32)Maximum_update_difftime));
                setLastUpdate(now);
            }
        }
        else
        {
            if (!m_client)
            {
                updateRotation();
            }
            else
            {
                ClientMoveState state;
                while (m_moveEvents.Next(state, now))
                    ApplyState(state);

                m_client->_OnUpdate();
            }
            setLastUpdate(now);
        }
    }

    void UnitMovementImpl::LaunchMoveSpline(MoveSplineInitArgs& args)
    {
        if (!HasUpdater())
        {
            log_console("UnitMovement::LaunchMoveSpline: attempt to lauch not initialized movement");
            return;
        }

        UnitMoveFlag moveFlag_new;
        PrepareMoveSplineArgs(args, moveFlag_new);
      
        if (!MoveSpline::Initialize(move_spline, args))
        {
            log_console("UnitMovement::LaunchMoveSpline: can't lauch, invalid movespline args");
            return;
        }

        SetMoveFlag(moveFlag_new | UnitMoveFlag::Spline_Enabled);
        //setLastUpdate(GetUpdater().TickTime());
        //updatable.ScheduleUpdate();

        PacketBuilder::SplinePathSend(*this, MsgBroadcast(this));
    }

    void UnitMovementImpl::PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new) const
    {
        args.path[0] = GetPosition3();    //correct first vertex
        args.splineId = GetUpdater().NewMoveSplineId();

        moveFlag_new = moveFlags & ~(UnitMoveFlag::Mask_Directions | UnitMoveFlag::Mask_Moving) | UnitMoveFlag::Spline_Enabled;
        moveFlag_new.backward = args.flags.backward;
        moveFlag_new.forward = !args.flags.backward && !args.flags.falling;
        moveFlag_new.walk_mode = args.flags.walkmode;

        // select velocity if was not set in SetVelocity
        if (args.velocity == 0.f)
            args.velocity = GetSpeed(UnitMovementImpl::SelectSpeedType(moveFlag_new));
    }

    std::string UnitMovementImpl::ToString() const
    {
        std::stringstream st;
        st << ClientState().ToString();
        if (m_moveEvents.Size() != 0)
            st << "states count: " << m_moveEvents.Size() << std::endl;

        if (m_client)
            st << m_client->ToString();
        if (SplineEnabled())
            st << move_spline->ToString();
        return st.str();
    }

    uint32 UnitMovementImpl::MoveSplineId() const
    {
        if (SplineEnabled())
            return move_spline->GetId();
        else
            return 0;
    }

    const Vector3& UnitMovementImpl::MoveSplineDest() const
    {
        if (SplineEnabled())
            return move_spline->FinalDestination();
        else
            return GetPosition3();
    }

    int32 UnitMovementImpl::MoveSplineTimeElapsed() const
    {
        if (SplineEnabled())
            return move_spline->timeElapsed();
        else
            return 0;
    }

    void UnitMovementImpl::SetMoveFlag(const UnitMoveFlag& newFlags)
    {
        if ((moveFlags & UnitMoveFlag::Mask_Speed) != (newFlags & UnitMoveFlag::Mask_Speed))
        {
            SpeedType speed_type = UnitMovementImpl::SelectSpeedType(newFlags);
            m_float_values[Parameter_SpeedCurrent] = GetSpeed(speed_type);
        }
        moveFlags = newFlags;
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
