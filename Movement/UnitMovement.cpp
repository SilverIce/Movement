
#include "UnitMovement.h"
#include "Object.h"
#include "moveupdater.h"
#include "MoveSpline.h"
#include "ClientMovement.h"
#include "MoveSplineInit.h"
#include "MovementMessage.h"

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
            2.0f,                                                   // CollisionHeight
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
        speed_type = speed_type_new;
        moveFlags = moveFlag_new;

        move_spline.Initialize(args);
        updatable.ScheduleUpdate();

        SetParameter(Parameter_SpeedCurrent, args.velocity);
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

    ClientMoveState UnitMovement::ClientState() const
    {
        ClientMoveState state;
        static_cast<_ClientMoveState>(state) = m_unused;
        state.ms_time = getLastUpdate();
        state.world_position = GetGlobalPosition();
        state.moveFlags = moveFlags;

        if (IsBoarded())
        {
            state.t_guid = GetTransport()->Owner.GetObjectGuid().GetRawValue();
            state.transport_position = GetLocalPosition();
        }
        return state;
    }

    struct ReqRespMsg
    {
        uint16 smsg_request;
        uint16 cmsg_response;
        uint16 msg;
        uint16 smsg_spline;
    };

    /* request-response-msg order*/
    #define VALUE_CHANGE(mode)   {SMSG_FORCE_##mode##_CHANGE, CMSG_FORCE_##mode##_CHANGE_ACK, MSG_MOVE_SET_##mode,SMSG_SPLINE_SET_##mode},
    static const ReqRespMsg ValueChange2Opc_table[UnitMovement::Parameter_End] =
    {
        VALUE_CHANGE(WALK_SPEED)
        VALUE_CHANGE(RUN_SPEED)
        VALUE_CHANGE(SWIM_BACK_SPEED)
        VALUE_CHANGE(SWIM_SPEED)
        VALUE_CHANGE(RUN_BACK_SPEED)
        VALUE_CHANGE(FLIGHT_SPEED)
        VALUE_CHANGE(FLIGHT_BACK_SPEED)
        VALUE_CHANGE(TURN_RATE)
        VALUE_CHANGE(PITCH_RATE)
        {SMSG_MOVE_SET_COLLISION_HGT,CMSG_MOVE_SET_COLLISION_HGT_ACK,MSG_MOVE_SET_COLLISION_HGT,0},
    };
    #undef CLIENT_VALUE_CHANGE

    class FloatValueChangeRequest : public RespHandler
    {
        uint32 m_reqId;
        UnitMovement::FloatParameter m_value_type;
        float m_value;

        FloatValueChangeRequest(Client * client, UnitMovement::FloatParameter value_type, float value) :
              RespHandler(ValueChange2Opc_table[value_type].cmsg_response),
              m_value_type(value_type),
              m_reqId(client->AddRespHandler(this)),
              m_value(value)
        { 
            if (uint16 opcode = ValueChange2Opc_table[value_type].smsg_request)
            {
                WorldPacket data(opcode, 32);
                data << client->controlled()->Owner.GetPackGUID();
                data << m_reqId;
                if (m_value_type == UnitMovement::Parameter_SpeedRun)
                    data << int8(0);                               // new 2.1.0
                data << m_value;
                client->SendPacket(data);
            }
        }

    public:

        static void Launch(UnitMovement * mov, UnitMovement::FloatParameter value_type, float value)
        {
            if (mov->IsClientControlled())
            {
                new FloatValueChangeRequest(mov->client(), value_type, value);
            }
            else
            {
                // FIXME: currently there is no way to change speed of already moving server-side controlled unit (spline movement)
                // there is only one hacky way - launch new spline movement.. that's how blizz doing this
                // if (mov->SplineEnabled())
                mov->SetParameter(value_type, value);
                if (uint16 opcode = ValueChange2Opc_table[value_type].smsg_spline)
                {
                    WorldPacket data(opcode, 16);
                    data << mov->Owner.GetPackGUID();
                    data << value;
                    MaNGOS_API::BroadcastMessage(&mov->Owner, data);
                }
            }
        }

        virtual void OnReply(Client * client, WorldPacket& data) override
        {
            ClientMoveState client_state;
            ObjectGuid guid;
            uint32 client_req_id;
            float client_value;
            data >> guid.ReadAsPacked();
            data >> client_req_id;
            data >> client_state;
            data >> client_value;
            if (client_req_id != m_reqId)
            {
                log_write("FloatValueChangeRequest::OnReply: wrong counter value: %u and should be: %u",client_req_id,m_reqId);
                return;
            }
            if (client_value != m_value)
            {
                log_write("FloatValueChangeRequest::OnReply: wrong float value(type %u): %f and should be: %f",m_value_type,client_value,m_value);
                return;
            }
            client->QueueState(client_state);
            client->controlled()->SetParameter(m_value_type, m_value);
            if (uint16 opcode = ValueChange2Opc_table[m_value_type].msg)
            {
                MovementMessage msg(client->controlled(), opcode, 64);
                msg << guid.WriteAsPacked();
                msg << client_state;
                msg << m_value;
                client->BroadcastMessage(msg);
            }
        }
    };

    void UnitMovement::SetCollisionHeight(float value)
    {
        FloatValueChangeRequest::Launch(this, Parameter_CollisionHeight, value);
    }

    void UnitMovement::SetSpeed(SpeedType type, float value)
    {
        if (GetSpeed(type) != value)
            FloatValueChangeRequest::Launch(this, (FloatParameter)type, value);
    }

    struct ModeInfo
    {
        UnitMoveFlag::eUnitMoveFlags moveFlag;
        uint16 smsg_apply[2];   // 0 is apply, 1 - unapply
        uint16 cmsg_ack[2];
        uint16 msg_apply[2];   // 0 is apply, 1 - unapply
        uint16 smsg_spline_apply[2];   // 0 is apply, 1 - unapply
    };

    #define CLIENT_MODE_CHANGE(apply,unapply,ack,msg_apply)\
            {SMSG_MOVE_##apply,SMSG_MOVE_##unapply,CMSG_MOVE_##ack##_ACK,MSG_MOVE_##msg_apply,MSG_MOVE_##msg_apply},\

    const ModeInfo modeInfo[MoveModeMaxCount]=
    {
        {
            UnitMoveFlag::Walk_Mode, 0, 0, 0, 0, 0, 0,
            SMSG_SPLINE_MOVE_SET_WALK_MODE, SMSG_SPLINE_MOVE_SET_RUN_MODE
        },
        {
            UnitMoveFlag::Root, SMSG_FORCE_MOVE_ROOT, SMSG_FORCE_MOVE_UNROOT,
            CMSG_FORCE_MOVE_ROOT_ACK, CMSG_FORCE_MOVE_ROOT_ACK,
            MSG_MOVE_ROOT, MSG_MOVE_UNROOT,
            SMSG_SPLINE_MOVE_ROOT, SMSG_SPLINE_MOVE_UNROOT
        },
        {
            UnitMoveFlag::Swimming, 0, 0, 0, 0, 0, 0,
            SMSG_SPLINE_MOVE_START_SWIM, SMSG_SPLINE_MOVE_STOP_SWIM
        },
        {
            UnitMoveFlag::Waterwalking, SMSG_MOVE_WATER_WALK, SMSG_MOVE_LAND_WALK,
            CMSG_MOVE_WATER_WALK_ACK, CMSG_MOVE_WATER_WALK_ACK,
            MSG_MOVE_WATER_WALK, MSG_MOVE_WATER_WALK,
            SMSG_SPLINE_MOVE_WATER_WALK, SMSG_SPLINE_MOVE_LAND_WALK
        },
        {
            UnitMoveFlag::Can_Safe_Fall, SMSG_MOVE_FEATHER_FALL, SMSG_MOVE_NORMAL_FALL,
            CMSG_MOVE_FEATHER_FALL_ACK, CMSG_MOVE_FEATHER_FALL_ACK,
            MSG_MOVE_FEATHER_FALL, MSG_MOVE_FEATHER_FALL,
            SMSG_SPLINE_MOVE_FEATHER_FALL, SMSG_SPLINE_MOVE_NORMAL_FALL
        },
        {
            UnitMoveFlag::Hover, SMSG_MOVE_SET_HOVER, SMSG_MOVE_UNSET_HOVER,
            CMSG_MOVE_HOVER_ACK, CMSG_MOVE_HOVER_ACK,
            MSG_MOVE_HOVER, MSG_MOVE_HOVER,
            SMSG_SPLINE_MOVE_SET_HOVER, SMSG_SPLINE_MOVE_UNSET_HOVER
        },
        {
            UnitMoveFlag::Flying, 0, 0, 0, 0, 0,
            SMSG_SPLINE_MOVE_SET_FLYING, SMSG_SPLINE_MOVE_UNSET_FLYING
        },
        {
            UnitMoveFlag::GravityDisabled, SMSG_MOVE_GRAVITY_DISABLE, SMSG_MOVE_GRAVITY_ENABLE,
            CMSG_MOVE_GRAVITY_DISABLE_ACK, CMSG_MOVE_GRAVITY_ENABLE_ACK,
            MSG_MOVE_GRAVITY_CHNG, MSG_MOVE_GRAVITY_CHNG,
            SMSG_SPLINE_MOVE_GRAVITY_DISABLE, SMSG_SPLINE_MOVE_GRAVITY_ENABLE
        },
        {
            UnitMoveFlag::Can_Fly, SMSG_MOVE_SET_CAN_FLY, SMSG_MOVE_UNSET_CAN_FLY,
            CMSG_MOVE_SET_CAN_FLY_ACK, CMSG_MOVE_SET_CAN_FLY_ACK,
            MSG_MOVE_UPDATE_CAN_FLY, MSG_MOVE_UPDATE_CAN_FLY,
            SMSG_SPLINE_MOVE_SET_FLYING, SMSG_SPLINE_MOVE_UNSET_FLYING
        },
    };

    class ModeChangeRequest : public RespHandler
    {
        uint32 m_reqId;
        MoveMode m_mode;
        bool m_apply;

        ModeChangeRequest(Client * client, MoveMode mode, bool apply) : RespHandler(modeInfo[mode].cmsg_ack[!apply]),
            m_mode(mode), m_apply(apply), m_reqId(client->AddRespHandler(this))
        {
            if (uint16 opcode = modeInfo[mode].smsg_apply[!apply])
            {
                MovementMessage msg(NULL, opcode, 16);
                msg << client->controlled()->Owner.GetPackGUID();
                msg << m_reqId;
                client->SendMoveMessage(msg);
            }
        }

    public:

        static void Launch(UnitMovement * mov, MoveMode mode, bool apply)
        {
            if (mov->IsClientControlled())
                new ModeChangeRequest(mov->client(), mode, apply);
            else if (uint16 opcode = modeInfo[mode].smsg_spline_apply[!apply])
            {
                WorldPacket data(opcode, 12);
                data << mov->Owner.GetPackGUID();
                MaNGOS_API::BroadcastMessage(&mov->Owner, data);
            }
        }

        virtual void OnReply(Client * client, WorldPacket& data) override
        {
            ClientMoveState client_state;
            ObjectGuid guid;
            uint32 client_req_id;

            data >> guid.ReadAsPacked();
            data >> client_req_id;
            data >> client_state;
            if (data.rpos() != data.size())
                data >> Unused<float>();          // 0 or 1, unused
            
            if (client_req_id != m_reqId)
            {
                log_write("FloatValueChangeRequest::OnReply: wrong counter value: %u and should be: %u",client_req_id,m_reqId);
                return;
            }
            if (modeInfo[m_mode].moveFlag != 0 && m_apply != (bool)(client_state.moveFlags & modeInfo[m_mode].moveFlag))
            {
                log_write("ModeChangeRequest::OnReply: wrong client's flag");
                return;
            }

            client->QueueState(client_state);

            if (uint16 opcode = modeInfo[m_mode].msg_apply[!m_apply])
            {
                MovementMessage msg(client->controlled(), opcode, 64);
                msg << guid.WriteAsPacked();
                msg << client_state;
                client->BroadcastMessage(msg);
            }
        }
    };

    void UnitMovement::ApplyMoveMode(MoveMode mode, bool apply)
    {
        ModeChangeRequest::Launch(this, mode, apply);
    }
}
