
#include "UnitMovement.h"
#include "Object.h"
#include "moveupdater.h"
#include "MoveSpline.h"
#include "ClientMovement.h"
#include "MoveSplineInit.h"
#include "packet_builder.h"

#include <sstream>

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
        if (m_state_queue.empty() || CurrentState().ms_time.time > time_now.time)
            return false;

        state = CurrentState();
        m_state_queue.pop_back();
        return true;
    }

    SpeedType UnitMovement::SelectSpeedType(UnitMoveFlag moveFlags)
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

    static const uint32 Mode2Flag_table[MoveModeMaxCount]=
    {
        {/*WALK_BEGAN,*/        UnitMoveFlag::Walk_Mode},
        {/*ROOT_BEGAN,*/        UnitMoveFlag::Root},
        {/*SWIM_BEGAN,*/        UnitMoveFlag::Swimming},
        {/*WATERWALK_MODE,*/    UnitMoveFlag::Waterwalking},
        {/*SLOW_FALL_BEGAN,*/   UnitMoveFlag::Can_Safe_Fall},
        {/*HOVER_BEGAN,*/       UnitMoveFlag::Hover},
        {/*FLY_BEGAN, */        UnitMoveFlag::Flying},
        {                       UnitMoveFlag::Levitating},
        {                       UnitMoveFlag::Can_Fly},
    };

    void UnitMovement::ApplyMoveMode( MoveMode mode, bool apply )
    {
        if (apply)
        {
            moveFlags |= Mode2Flag_table[mode];
            move_mode |= (1 << mode);
        }
        else
        {
            moveFlags &= ~Mode2Flag_table[mode];
            move_mode &= ~(1 << mode);
        }
    }


    UnitMovement::UnitMovement(WorldObjectType owner) :
        Transportable(owner), move_spline(*new MoveSpline()), m_transport(*this),
        m_client(NULL)
    {
        updatable.SetUpdateStrategy(this);
        reset_managed_position();

        move_mode = 0;

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
            7.0f,                                                   // SpeedCurrent
        };

        memcpy(m_float_values,BaseValues, sizeof m_float_values);
        speed_type = SpeedRun;
        dbg_flags = 0;
    }

    UnitMovement::~UnitMovement()
    {
        delete &move_spline;
    }

    void UnitMovement::CleanReferences()
    {
        if (m_client)
        {
            m_client->Dereference(this);
            m_client = NULL;
        }

        UnbindOrientation();
        m_transport.CleanReferences();
        Transportable::CleanReferences();
        updatable.CleanReferences();
    }

    void UnitMovement::ReCalculateCurrentSpeed()
    {
        speed_type = UnitMovement::SelectSpeedType(moveFlags);
        m_float_values[Parameter_SpeedCurrent] = GetSpeed(speed_type);
    }

    Vector3 UnitMovement::direction() const
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

    void UnitMovement::Initialize(const Location& pos, MoveUpdater& updater)
    {
        SetPosition(pos);
        updatable.SetUpdater(updater);
        setLastUpdate(GetUpdater().TickTime());
    }

    void UnitMovement::ApplyState(const ClientMoveState& new_state)
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

        moveFlags = new_flags;
        m_local_position = new_state.transport_position;
        m_unused = new_state;
        ReCalculateCurrentSpeed();
    }


    void UnitMovement::updateRotation()
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
        }
    */
    }

    void UnitMovement::BindOrientationTo(MovementBase& target)
    {
        UnbindOrientation();

        if (&target == this)
        {
            log_write("UnitMovement::BindOrientationTo: trying to target self, skipped");
            return;
        }

        // can i target self?
        m_target_link.Value = TargetLink(&target, this);
        target._link_targeter(m_target_link);
        Owner.SetUInt64Value(UNIT_FIELD_TARGET, target.Owner.GetGUID());
    }

    void UnitMovement::UnbindOrientation()
    {
        m_target_link.delink();
        m_target_link.Value = TargetLink();
        Owner.SetUInt64Value(UNIT_FIELD_TARGET, 0);
    }

    void UnitMovement::SetSpeed(SpeedType type, float s)
    {
        if (GetSpeed(type) != s)
        {
            SetParameter((FloatParameter)type, s);
            PacketBuilder::SpeedUpdate(*this, type, MsgBroadcast(this));

            // FIXME: currently there is no way to change speed of already moving server-side controlled unit (spline movement)
            // there is only one hacky way - launch new spline movement.. that's how blizz doing this
            /*if (SplineEnabled() && type == getCurrentSpeedType())
            {
                if (G3D::fuzzyEq(s,0))
                    Scketches(*this).ForceStop();
                else
                {
                    Scketches(*this).ForceStop();
                }
            }*/
        }
    }


    struct UnitMovement::MoveSplineUpdater
    {
        UnitMovement& mov;
        MoveSpline& move_spline;
        bool NeedSync;

        explicit MoveSplineUpdater(UnitMovement& movement, int32 difftime) :
            mov(movement), NeedSync(false), move_spline(mov.move_spline)
        {
            move_spline.updateState(difftime, *this);
            mov.SetPosition(move_spline.ComputePosition());

            if (NeedSync)
                PacketBuilder::SplineSyncSend(mov, MsgBroadcast(mov));
        }

        inline void operator()(MoveSpline::UpdateResult result)
        {
            switch (result)
            {
            case MoveSpline::Result_NextSegment:
                //log_console("UpdateState: segment %d is on hold, position: %s", move_spline.currentSplineSegment(),GetPosition3().toString().c_str());
                if (mov.listener)
                    mov.listener->OnEvent( OnEventArgs::OnPoint(move_spline.GetId(),move_spline.currentPathIdx()) );
                break;
            case MoveSpline::Result_Arrived:
                //log_console("UpdateState: spline done, position: %s", GetPosition3().toString().c_str());
                mov.DisableSpline();
                if (mov.listener)
                {
                    // it's never possible to have 'current point == last point', need send point+1 here
                    mov.listener->OnEvent( OnEventArgs::OnPoint(move_spline.GetId(),move_spline.currentPathIdx()+1) );
                    mov.listener->OnEvent( OnEventArgs::OnArrived(move_spline.GetId()) );
                }
                break;
            case MoveSpline::Result_NextCycle:
                NeedSync = true;
                break;
            }
        }
    };

    void UnitMovement::UpdateState()
    {
        MSTime now = GetUpdater().TickTime();

        if (SplineEnabled())
        {
            int32 difftime = (now - getLastUpdate()).time;
            if (move_spline.timeElapsed() <= difftime || difftime >= MoveSpline_UpdateDelay)
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

    void UnitMovement::BoardOn(Transport& transport, const Location& local_position, int8 seatId)
    {
        _board(transport, local_position);
        set_managed_position(m_local_position);

        m_unused.transport_seat = seatId;
        moveFlags.ontransport = true;
    }

    void UnitMovement::Unboard()
    {
        _unboard();
        reset_managed_position();

        m_unused.transport_seat = 0;
        moveFlags.ontransport = false;
    }

    void UnitMovement::SetPosition(const Location& v)
    {
        // dirty code..
        if (managed_position == &GetGlobalPosition())
            SetGlobalPosition(v);
        else
            *managed_position = v;
    }

    void UnitMovement::LaunchMoveSpline(MoveSplineInitArgs& args)
    {
        if (!HasUpdater())
        {
            log_console("UnitMovement::LaunchMoveSpline: attempt to lauch not initialized movement");
            return;
        }

        UnitMoveFlag moveFlag_new;
        SpeedType speed_type_new;
        PrepareMoveSplineArgs(args, moveFlag_new, speed_type_new);

        if (!args.Validate())
        {
            log_console("UnitMovement::LaunchMoveSpline: can't lauch, invalid movespline args");
            return;
        }

        setLastUpdate(GetUpdater().TickTime());
        speed_obj.current = args.velocity;
        speed_type = speed_type_new;
        moveFlags = moveFlag_new;

        move_spline.Initialize(args);
        updatable.ScheduleUpdate();

        SetControl(MovControlServer);

        PacketBuilder::SplinePathSend(*this, MsgBroadcast(this));
    }

    void UnitMovement::PrepareMoveSplineArgs(MoveSplineInitArgs& args, UnitMoveFlag& moveFlag_new, SpeedType& speed_type_new) const
    {
        args.path[0] = GetPosition3();    //correct first vertex
        args.splineId = GetUpdater().NewMoveSplineId();

        moveFlag_new = moveFlags & ~(UnitMoveFlag::Mask_Directions | UnitMoveFlag::Mask_Moving) | UnitMoveFlag::Spline_Enabled;
        moveFlag_new.backward = args.flags.backward;
        moveFlag_new.forward = !args.flags.backward && !args.flags.falling;
        moveFlag_new.walk_mode = args.flags.walkmode;

        // select velocity if was not set in SetVelocity
        if (args.velocity == 0.f)
        {
            speed_type_new = UnitMovement::SelectSpeedType(moveFlag_new);
            args.velocity = GetSpeed(speed_type_new);
        }
        else
            speed_type_new = SpeedNotStandart;
    }

    std::string UnitMovement::ToString() const
    {
        std::stringstream st;
        st << "Movement  flags: " << moveFlags.ToString() << std::endl;
        st << "Global position: " << GetGlobalPosition().toString() << std::endl;

        if (moveFlags.ontransport)
            st << "Local  position: " << GetPosition().toString() << std::endl;

        if (moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || m_unused.moveFlags2.allow_pitching)
        {
            st << "pitch angle " << m_unused.pitch << std::endl;
        }

        if (moveFlags.falling)
        {
            st << "jump z  vel " << m_unused.jump_velocity << std::endl;
            st << "jump    sin " << m_unused.jump_sinAngle << std::endl;
            st << "jump    cos " << m_unused.jump_cosAngle << std::endl;
            st << "jump xy vel " << m_unused.jump_xy_velocy << std::endl;
        }

        if (m_moveEvents.Size() != 0)
            st << "states count: " << m_moveEvents.Size() << std::endl;

        if (m_client)
            st << m_client->ToString();

        if (SplineEnabled())
            st << move_spline.ToString();

        return st.str();
    }

    uint32 UnitMovement::MoveSplineId() const
    {
        if (SplineEnabled())
            return move_spline.GetId();
        else
            return 0;
    }

    const Vector3& UnitMovement::MoveSplineDest() const
    {
        if (SplineEnabled())
            return move_spline.FinalDestination();
        else
            return GetPosition3();
    }

    int32 UnitMovement::MoveSplineTimeElapsed() const
    {
        if (SplineEnabled())
            return move_spline.timeElapsed();
        else
            return 0;
    }
}
