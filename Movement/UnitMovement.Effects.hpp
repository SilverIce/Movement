#pragma once

namespace Movement
{
    struct ReqRespMsg
    {
        ClientOpcode smsg_request;
        ClientOpcode cmsg_response;
        ClientOpcode msg;
        ClientOpcode smsg_spline;
    };

    /* request-response-msg order*/
#define VALUE_CHANGE(mode)   {SMSG_FORCE_##mode##_CHANGE, CMSG_FORCE_##mode##_CHANGE_ACK, MSG_MOVE_SET_##mode,SMSG_SPLINE_SET_##mode},
    static const ReqRespMsg ValueChange2Opc_table[Parameter_End] =
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
        {SMSG_MOVE_SET_COLLISION_HGT,CMSG_MOVE_SET_COLLISION_HGT_ACK,MSG_MOVE_SET_COLLISION_HGT,MSG_NULL_ACTION},
        {MSG_NULL_ACTION,MSG_NULL_ACTION,MSG_NULL_ACTION,MSG_NULL_ACTION},
    };
#undef VALUE_CHANGE

    class FloatValueChangeEffect : private RespHandler
    {
        FloatParameter m_value_type;
        float m_value;

        FloatValueChangeEffect(ClientImpl * client, FloatParameter value_type, float value) :
            RespHandler(ValueChange2Opc_table[value_type].cmsg_response, client),
            m_value_type(value_type),
            m_value(value)
        {
            if (ClientOpcode opcode = ValueChange2Opc_table[value_type].smsg_request)
            {
                WorldPacket data(opcode, 32);
                data << client->controlled()->Guid.WriteAsPacked();
                data << m_requestId;
                if (m_value_type == Parameter_SpeedRun)
                    data << int8(0);                               // new 2.1.0
                data << m_value;
                client->SendPacket(data);
            }
        }

    public:

        static void Launch(UnitMovementImpl * mov, FloatParameter value_type, float value)
        {
            if (mov->IsClientControlled())
            {
                new FloatValueChangeEffect(mov->client(), value_type, value);
            }
            else
            {
                mov->SetParameter(value_type, value);
                // FIXME: currently there is no way to change speed of already moving server-side controlled unit (spline movement)
                // there is only one hacky way - launch new spline movement.. that's how blizz doing this

                WorldPacket data(opcode, 16);
                data << mov->Guid.WriteAsPacked();
                data << value;
                Imports.BroadcastMessage(mov->Owner, data);
            }
        }

    private:
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
        {
            ClientMoveStateChange client_state;
            ObjectGuid guid;
            uint32 client_req_id;
            data >> guid.ReadAsPacked();
            data >> client_req_id;
            data >> client_state;
            data >> client_state.floatValue;
            if (!checkRequestId(client_req_id))
                return false;
            if (client_state.floatValue != m_value)
            {
                log_fatal("wrong float value(type %u): %f and should be: %f",m_value_type,client_state.floatValue,m_value);
                return false;
            }
            client_state.floatValueType = m_value_type;
            client->QueueState(client_state);
            if (ClientOpcode opcode = ValueChange2Opc_table[m_value_type].msg)
            {
                MovementMessage msg(client->controlled(), opcode, 64);
                msg << guid.WriteAsPacked();
                msg << client_state;
                msg << m_value;
                client->BroadcastMessage(msg);
            }
            return true;
        }
    };

    struct ModeInfo
    {
        UnitMoveFlag::eUnitMoveFlags moveFlag;
        ClientOpcode smsg_apply[2];   // 0 is apply, 1 - unapply
        ClientOpcode cmsg_ack[2];
        ClientOpcode msg_apply[2];   // 0 is apply, 1 - unapply
        ClientOpcode smsg_spline_apply[2];   // 0 is apply, 1 - unapply
    };

    const ModeInfo modeInfo[MoveMode_End]=
    {
        {
            UnitMoveFlag::Walk_Mode, MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
                SMSG_SPLINE_MOVE_SET_WALK_MODE, SMSG_SPLINE_MOVE_SET_RUN_MODE
        },
        {
            UnitMoveFlag::Root, SMSG_FORCE_MOVE_ROOT, SMSG_FORCE_MOVE_UNROOT,
                CMSG_FORCE_MOVE_ROOT_ACK, CMSG_FORCE_MOVE_UNROOT_ACK,
                MSG_MOVE_ROOT, MSG_MOVE_UNROOT,
                SMSG_SPLINE_MOVE_ROOT, SMSG_SPLINE_MOVE_UNROOT
        },
        {
            UnitMoveFlag::Swimming, MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
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
            UnitMoveFlag::Flying, MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
                MSG_NULL_ACTION, MSG_NULL_ACTION,
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
        {
            UnitMoveFlag::AllowSwimFlyTransition, SMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY, SMSG_MOVE_UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,
                CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK, CMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY_ACK,
                MSG_MOVE_UPDATE_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY, MSG_MOVE_UPDATE_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,
                MSG_NULL_ACTION, MSG_NULL_ACTION
        },
    };

    class ModeChangeEffect : private RespHandler
    {
        MoveMode m_mode;
        bool m_apply;

        ModeChangeEffect(ClientImpl * client, MoveMode mode, bool apply) :
            RespHandler(modeInfo[mode].cmsg_ack[!apply],client), m_mode(mode), m_apply(apply)
        {
            MovementMessage msg(NULL, modeInfo[mode].smsg_apply[!apply], 16);
            msg << client->controlled()->Guid.WriteAsPacked();
            msg << m_requestId;
            client->SendMoveMessage(msg);
        }

    public:

        static void Launch(UnitMovementImpl * mov, MoveMode mode, bool apply)
        {
            if (mov->IsClientControlled())
            {
                if (modeInfo[mode].smsg_apply[!apply])
                    new ModeChangeEffect(mov->client(), mode, apply);
                else
                    log_function("no opcode for mode %u", mode);
            }
            else
            {
                if (ClientOpcode opcode = modeInfo[mode].smsg_spline_apply[!apply])
                {
                    /** By some unknown reason client force moves unit to end of the path when receives
                        SMSG_SPLINE_MOVE_ROOT/SMSG_SPLINE_MOVE_UNROOT packet.
                        Need override current spine movement to avoid this. blizzs doing same hack. */
                    if (mode == MoveModeRoot && mov->SplineEnabled())
                        MoveSplineInit(*mov).Launch();

                    mov->ApplyMoveFlag(modeInfo[mode].moveFlag, apply);
                    WorldPacket data(opcode, 12);
                    data << mov->Guid.WriteAsPacked();
                    Imports.BroadcastMessage(mov->Owner, data);
                }
                else
                    log_function("no opcode for mode %u", mode);
            }
        }

    private:
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
        {
            ClientMoveStateChange client_state;
            ObjectGuid guid;
            uint32 client_req_id;

            data >> guid.ReadAsPacked();
            data >> client_req_id;
            data >> client_state;
            if (data.rpos() != data.size())
                data >> Unused<float>();          // 0 or 1, unused

            if (!checkRequestId(client_req_id))
                return false;

            client_state.allowFlagChange = modeInfo[m_mode].moveFlag;
            client_state.allowFlagApply = m_apply;
            client->QueueState(client_state);

            if (ClientOpcode opcode = modeInfo[m_mode].msg_apply[!m_apply])
            {
                MovementMessage msg(client->controlled(), opcode, 64);
                msg << guid.WriteAsPacked();
                msg << client_state;
                client->BroadcastMessage(msg);
            }
            return true;
        }
    };

    bool UnitMovementImpl::HasMode(MoveMode mode) const
    {
        return moveFlags.hasFlag(modeInfo[mode].moveFlag);
    }

    class TeleportEffect : private RespHandler
    {
        Location m_location;

        TeleportEffect(ClientImpl * client, const Location& loc) :
            RespHandler(MSG_MOVE_TELEPORT_ACK, client), m_location(loc)
        {
            ClientMoveState state(client->controlled()->ClientState());
            // TODO: add set of functions for state modifying
            if (state.moveFlags.ontransport)
                state.transport_position = m_location;
            else
                state.world_position = m_location;

            MovementMessage msg(NULL, MSG_MOVE_TELEPORT_ACK, 64);   // message source is null - client shouldn't skip that message
            msg << client->controlled()->Guid.WriteAsPacked();
            msg << m_requestId;
            msg << state;
            client->SendMoveMessage(msg);
        }

    public:

        static void Launch(UnitMovementImpl * mov, const Location& loc)
        {
            if (mov->IsClientControlled())
            {
                new TeleportEffect(mov->client(), loc);
            }
            else
            {
                MoveSplineInit init(*mov);
                init.MoveTo(loc);
                init.SetFacing(loc.orientation);
                init.SetInstant();
                init.Launch();
            }
        }

    private:
        virtual bool OnReply(ClientImpl * client, WorldPacket& data) override
        {
            ObjectGuid guid;
            uint32 client_req_id;
            MSTime client_time;
            data >> guid.ReadAsPacked();
            data >> client_req_id;
            data >> client_time;

            if (!checkRequestId(client_req_id))
                return false;

            client->controlled()->SetRelativePosition(m_location);

            MovementMessage msg(client->controlled(), MSG_MOVE_TELEPORT, 64);
            msg << guid.WriteAsPacked();
            msg << client->controlled()->ClientState();
            client->BroadcastMessage(msg);
            return true;
        }
    };

    class KnockbackEffect : private RespHandler
    {
        Vector2 m_direction2d;
        float m_horizontalVelocity;
        float m_verticalVelocity;

        bool OnReply(ClientImpl * client, WorldPacket& data) override
        {
            ObjectGuid guid;
            uint32 requestId;
            ClientMoveStateChange state;

            data >> guid.ReadAsPacked();
            data >> requestId;
            data >> state;

            if (!checkRequestId(requestId))
                return false;

            if (m_direction2d.x != state.jump_directionX ||
                m_direction2d.y != state.jump_directionY ||
                m_horizontalVelocity != state.jump_horizontalVelocity ||
                m_verticalVelocity != state.jump_verticalVelocity
                )
            {
                log_function("client's movement state has some invalid properties");
                return false;
            }

            // client disables Can_Fly flag
            // all these checks make my code a bit difficult and are dependant on client code
            state.allowFlagChange = UnitMoveFlag::Can_Fly;
            state.allowFlagApply = false;

            client->QueueState(state);

            MovementMessage msg(client->controlled(), MSG_MOVE_KNOCK_BACK, data.size() + 16);
            msg << guid.WriteAsPacked();
            msg << state;
            msg << state.jump_directionY;
            msg << state.jump_directionX;
            msg << state.jump_horizontalVelocity;
            msg << state.jump_verticalVelocity;
            client->BroadcastMessage(msg);
            return true;
        }

        KnockbackEffect(ClientImpl& client, float directionAngle, float horizontalVelocity, float verticalVelocity)
            : RespHandler(CMSG_MOVE_KNOCK_BACK_ACK, &client)
        {
            m_direction2d.x = cos(directionAngle);
            m_direction2d.y = sin(directionAngle);
            m_horizontalVelocity = horizontalVelocity;
            // inverse 'verticalVelocity' sign: user's input is positive value
            // in WoW all landing forces has positive sign and all lift off forces - negative
            m_verticalVelocity = -verticalVelocity;

            WorldPacket data(SMSG_MOVE_KNOCK_BACK, 32);
            data << client.controlled()->Guid.WriteAsPacked();
            data << m_requestId;
            data << m_direction2d.x;
            data << m_direction2d.y;
            data << m_horizontalVelocity;
            data << m_verticalVelocity;
            client.SendPacket(data);
        }

    public:

        static void Launch(UnitMovementImpl& movement, float directionAngle, float horizontalVelocity, float verticalVelocity)
        {
            if (movement.IsClientControlled())
                new KnockbackEffect(*movement.client(), directionAngle, horizontalVelocity, verticalVelocity);
            else
            {
                float moveTimeHalf = verticalVelocity / (float)Gravity();
                float maxAmplitude = -computeFallElevation(moveTimeHalf,-verticalVelocity);
                // TODO: correct destination to not make unit fall to void
                Vector3 destination = movement.GetRelativePosition() +
                    2.f * moveTimeHalf * horizontalVelocity * Vector3(cos(directionAngle),sin(directionAngle),0.f);

                MoveSplineInit init(movement);
                init.MoveTo(destination);
                init.SetParabolic(maxAmplitude, 0.f);
                init.SetOrientationFixed(true);
                init.SetVelocity(horizontalVelocity);
                init.Launch();
            }
        }
    };

    //////////////////////////////////////////////////////////////////////////

    MoveHandlersBinder::MoveHandlersBinder()
    {

#define ASSIGN_HANDLER(MessageHanger, ... ) { \
    ClientOpcode opcodes[] = {__VA_ARGS__}; \
    assignHandler(MessageHanger, opcodes, CountOf(opcodes)); \
    }
        assignHandler(&ClientImpl::OnMoveTimeSkipped, CMSG_MOVE_TIME_SKIPPED);

        for (uint32 i = 0; i < CountOf(ValueChange2Opc_table); ++i)
            assignHandler(&RespHandler::OnResponse, ValueChange2Opc_table[i].cmsg_response);

        for (uint32 i = 0; i < CountOf(modeInfo); ++i) {
            assignHandler(&RespHandler::OnResponse, modeInfo[i].cmsg_ack[0]);
            assignHandler(&RespHandler::OnResponse, modeInfo[i].cmsg_ack[1]);
        }

        ASSIGN_HANDLER(&RespHandler::OnResponse,
            CMSG_TIME_SYNC_RESP,
            CMSG_MOVE_KNOCK_BACK_ACK,
            MSG_MOVE_TELEPORT_ACK);

        ASSIGN_HANDLER(&ClientImpl::OnCommonMoveMessage,
            MSG_MOVE_START_FORWARD,
            MSG_MOVE_START_BACKWARD,
            MSG_MOVE_STOP,
            MSG_MOVE_START_STRAFE_LEFT,
            MSG_MOVE_START_STRAFE_RIGHT,
            MSG_MOVE_STOP_STRAFE,
            MSG_MOVE_JUMP,
            MSG_MOVE_START_TURN_LEFT,
            MSG_MOVE_START_TURN_RIGHT,
            MSG_MOVE_STOP_TURN,
            MSG_MOVE_START_PITCH_UP,
            MSG_MOVE_START_PITCH_DOWN,
            MSG_MOVE_STOP_PITCH,
            MSG_MOVE_SET_RUN_MODE,
            MSG_MOVE_SET_WALK_MODE,
            MSG_MOVE_FALL_LAND,
            MSG_MOVE_START_SWIM,
            MSG_MOVE_STOP_SWIM,
            MSG_MOVE_SET_FACING,
            MSG_MOVE_SET_PITCH,
            MSG_MOVE_HEARTBEAT,
            CMSG_MOVE_FALL_RESET,
            CMSG_MOVE_SET_FLY,
            MSG_MOVE_START_ASCEND,
            MSG_MOVE_STOP_ASCEND,
            CMSG_MOVE_CHNG_TRANSPORT,
            MSG_MOVE_START_DESCEND);

        assignHandler(&ClientImpl::OnSplineDone, CMSG_MOVE_SPLINE_DONE);
        assignHandler(&ClientImpl::OnNotActiveMover, CMSG_MOVE_NOT_ACTIVE_MOVER);
        assignHandler(&ClientImpl::OnActiveMover, CMSG_SET_ACTIVE_MOVER);

#undef ASSIGN_HANDLER
    }
}
