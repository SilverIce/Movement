
#include "UnitMovement.h"
#include "MoveSpline.h"
#include "MovementMessage.h"
#include "Object.h"
#include "MoveUpdater.h"

namespace Movement
{

    const uint16 S_Mode2Opc_table[MoveModeMaxCount][2]=
    {
        {/*WALK_BEGAN,*/        SMSG_SPLINE_MOVE_SET_RUN_MODE, SMSG_SPLINE_MOVE_SET_WALK_MODE},
        {/*ROOT_BEGAN,*/        SMSG_SPLINE_MOVE_UNROOT,       SMSG_SPLINE_MOVE_ROOT},
        {/*SWIM_BEGAN,*/        SMSG_SPLINE_MOVE_STOP_SWIM,    SMSG_SPLINE_MOVE_START_SWIM},
        {/*WATERWALK_MODE,*/    SMSG_SPLINE_MOVE_LAND_WALK,    SMSG_SPLINE_MOVE_WATER_WALK},
        {/*SLOW_FALL_BEGAN,*/   SMSG_SPLINE_MOVE_NORMAL_FALL,  SMSG_SPLINE_MOVE_FEATHER_FALL},
        {/*HOVER_BEGAN,*/       SMSG_SPLINE_MOVE_UNSET_HOVER,  SMSG_SPLINE_MOVE_SET_HOVER},
        {/*FLY_BEGAN,*/         SMSG_SPLINE_MOVE_UNSET_FLYING, SMSG_SPLINE_MOVE_SET_FLYING},
        {/*levitation mode*/    MSG_NULL_ACTION,               MSG_NULL_ACTION},    // no opcodes
        {/*can fly mode*/       SMSG_SPLINE_MOVE_UNSET_FLYING, SMSG_SPLINE_MOVE_SET_FLYING},    // no opcodes ?
    };

    const uint16 C_Mode2Opc_table[MoveModeMaxCount][2]=
    {
        {/*WALK_BEGAN,*/        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/*ROOT_BEGAN,*/        SMSG_FORCE_MOVE_UNROOT,        SMSG_FORCE_MOVE_ROOT},
        {/*SWIM_BEGAN,*/        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/*WATERWALK_MODE,*/    SMSG_MOVE_LAND_WALK,           SMSG_MOVE_WATER_WALK},
        {/*SLOW_FALL_BEGAN,*/   SMSG_MOVE_NORMAL_FALL,         SMSG_MOVE_FEATHER_FALL},
        {/*HOVER_BEGAN,*/       SMSG_MOVE_UNSET_HOVER,         SMSG_MOVE_SET_HOVER},
        {/*FLY_BEGAN,*/         SMSG_MOVE_UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,        SMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY},
        {/*levitation mode*/    MSG_NULL_ACTION,               MSG_NULL_ACTION},    // no opcodes
        {/*can fly mode*/       SMSG_MOVE_UNSET_CAN_FLY,       SMSG_MOVE_SET_CAN_FLY},
    };

/* change-ack-msg order*/
#define CLIENT_VALUE_CHANGE(mode)   {SMSG_FORCE_##mode##_CHANGE, CMSG_FORCE_##mode##_CHANGE_ACK, MSG_MOVE_SET_##mode},

    const uint16 C_Speed2Opc_table[SpeedMaxCount][3]=
    {
        CLIENT_VALUE_CHANGE(WALK_SPEED)
        CLIENT_VALUE_CHANGE(RUN_SPEED)
        CLIENT_VALUE_CHANGE(SWIM_BACK_SPEED)
        CLIENT_VALUE_CHANGE(RUN_BACK_SPEED)
        CLIENT_VALUE_CHANGE(FLIGHT_SPEED)
        CLIENT_VALUE_CHANGE(FLIGHT_BACK_SPEED)
        CLIENT_VALUE_CHANGE(TURN_RATE)
        CLIENT_VALUE_CHANGE(PITCH_RATE)
    };

#define SERVER_SPEED_CHANGE(mode)   {SMSG_SPLINE_SET_##mode},

    const uint16 S_Speed2Opc_table[SpeedMaxCount]=
    {
        SERVER_SPEED_CHANGE(WALK_SPEED)
        SERVER_SPEED_CHANGE(RUN_SPEED)
        SERVER_SPEED_CHANGE(SWIM_BACK_SPEED)
        SERVER_SPEED_CHANGE(RUN_BACK_SPEED)
        SERVER_SPEED_CHANGE(FLIGHT_SPEED)
        SERVER_SPEED_CHANGE(FLIGHT_BACK_SPEED)
        SERVER_SPEED_CHANGE(TURN_RATE)
        SERVER_SPEED_CHANGE(PITCH_RATE)
    };

/* change-ack-msg order*/
#define CLIENT_MODE_CHANGE(apply,unapply,ack,msg)   {SMSG_MOVE_##apply, SMSG_MOVE_##unapply, CMSG_MOVE_##ack##_ACK, MSG_MOVE_##msg},

    const uint16 __C_Mode2Opc_table[8][4]=
    {
       // {SMSG_FORCE_MOVE_ROOT, SMSG_FORCE_MOVE_UNROOT, CMSG_FORCE_MOVE_ROOT_ACK, MSG_MOVE_ROOT,MSG_MOVE_UNROOT},
        CLIENT_MODE_CHANGE(SET_CAN_FLY,UNSET_CAN_FLY,SET_CAN_FLY,UPDATE_CAN_FLY)
        CLIENT_MODE_CHANGE(SET_HOVER,UNSET_HOVER,HOVER,HOVER)
        CLIENT_MODE_CHANGE(WATER_WALK,LAND_WALK,WATER_WALK,WATER_WALK)
        CLIENT_MODE_CHANGE(FEATHER_FALL,NORMAL_FALL,FEATHER_FALL,FEATHER_FALL)
        CLIENT_MODE_CHANGE(SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY, UPDATE_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY)

        CLIENT_MODE_CHANGE(KNOCK_BACK,KNOCK_BACK,KNOCK_BACK,KNOCK_BACK)
        CLIENT_MODE_CHANGE(SET_COLLISION_HGT,SET_COLLISION_HGT,SET_COLLISION_HGT,SET_COLLISION_HGT)

        CLIENT_MODE_CHANGE(GRAVITY_ENABLE, GRAVITY_DISABLE, GRAVITY_ENABLE, GRAVITY_CHNG)


        


/*
        {/ *WALK_BEGAN,* /        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/ *ROOT_BEGAN,* /        SMSG_FORCE_MOVE_UNROOT,        SMSG_FORCE_MOVE_ROOT},
        {/ *SWIM_BEGAN,* /        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/ *WATERWALK_MODE,* /    SMSG_MOVE_LAND_WALK,           SMSG_MOVE_WATER_WALK},
        {/ *SLOW_FALL_BEGAN,* /   SMSG_MOVE_NORMAL_FALL,         SMSG_MOVE_FEATHER_FALL},
        {/ *HOVER_BEGAN,* /       SMSG_MOVE_UNSET_HOVER,         SMSG_MOVE_SET_HOVER},
        {/ *FLY_BEGAN,* /         SMSG_MOVE_UNSET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY,        SMSG_MOVE_SET_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY},
        {/ *levitation mode* /    MSG_NULL_ACTION,               MSG_NULL_ACTION},    // no opcodes
        {/ *can fly mode* /       SMSG_MOVE_UNSET_CAN_FLY,       SMSG_MOVE_SET_CAN_FLY},
*/
    };


    typedef void (*SpeedPtr)(const UnitMovement&,SpeedType,WorldPacket&);
    typedef void (*MoveModePtr)(const UnitMovement&,MoveMode,WorldPacket&);
    typedef void (*PathPtr)(const UnitMovement&,WorldPacket&);

    void PacketBuilder::SpeedUpdate(const UnitMovement& mov, SpeedType type, MsgDeliverer& broadcast)
    {
        static const SpeedPtr speed_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_SpeedUpdate,
            &PacketBuilder::Spline_SpeedUpdate,
        };

        WorldPacket data(MSG_NULL_ACTION, 64);
        speed_ptrs[mov.GetControl()](mov, type, data);

        if (data.GetOpcode() != MSG_NULL_ACTION)
            broadcast(data);
    }

    void PacketBuilder::MoveModeUpdate(const UnitMovement& mov, MoveMode move_mode, MsgDeliverer& broadcast)
    {
        static const MoveModePtr move_mode_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_MoveModeUpdate,
            &PacketBuilder::Spline_MoveModeUpdate,
        };

        WorldPacket data(MSG_NULL_ACTION, 64);
        move_mode_ptrs[mov.GetControl()](mov, move_mode, data);

        if (data.GetOpcode() != MSG_NULL_ACTION)
            broadcast(data);
    }

    void PacketBuilder::Spline_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket& data)
    {
        uint16 opcode = S_Speed2Opc_table[type];

        data.SetOpcode(opcode);
        data << mov.Owner.GetPackGUID();
        data << mov.GetSpeed(type);
    }

    void PacketBuilder::Spline_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket& data)
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];

        data.SetOpcode(opcode);
        data << mov.Owner.GetPackGUID();
    }

    void PacketBuilder::WriteCommonMonsterMovePart(const UnitMovement& mov, WorldPacket& data)
    {
        const MoveSpline& move_spline = mov.move_spline;
        MoveSplineFlag splineflags = move_spline.splineflags;

        if (mov.IsBoarded())
        {
            data.SetOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
            data << mov.Owner.GetPackGUID();
            data << mov.GetTransport()->Owner.GetPackGUID();
            data << int8(mov.m_unused.transport_seat);
        }
        else
        {
            data.SetOpcode(SMSG_MONSTER_MOVE);
            data << mov.Owner.GetPackGUID();
        }

        data << uint8(0);
        data << mov.GetPosition3();
        data << move_spline.GetId();

        switch(splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
        default:
            data << uint8(MonsterMoveNormal);
            break;
        case MoveSplineFlag::Final_Target:
            data << uint8(MonsterMoveFacingTarget);
            data << move_spline.facing.target;
            break;
        case MoveSplineFlag::Final_Angle:
            data << uint8(MonsterMoveFacingAngle);
            data << move_spline.facing.angle;
            break;
        case MoveSplineFlag::Final_Point:
            data << uint8(MonsterMoveFacingSpot);
            data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        splineflags.enter_cycle = move_spline.isCyclic();
        data << uint32(splineflags & ~MoveSplineFlag::Mask_No_Monster_Move);

        if (splineflags.animation)
        {
            data << splineflags.getAnimationId();
            data << move_spline.effect_start_time;
        }

        data << move_spline.Duration();

        if (splineflags.parabolic)
        {
            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;
        }
    }

    void PacketBuilder::WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.getPointCount() - 3;
        const Vector3 * real_path = &spline.getPoint(1);

        data << last_idx;
        data << real_path[last_idx];   // destination
        if (last_idx > 1)
        {
            Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
            Vector3 offset;
            // first and last points already appended
            for(uint32 i = 1; i < last_idx; ++i)
            {
                offset = middle - real_path[i];
                data.appendPackXYZ(offset.x, offset.y, offset.z);
            }
        }
    }

    void PacketBuilder::WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << count;
        data.append<Vector3>(&spline.getPoint(2), count);
    }

    /*
    Mover GUID: 0x079A1 Unit 1E4B
    unk byte: 0
    Current Position: 6557.049 1156.459 385.3375
    sequence Id: 62345736
    MovementType: NORMAL
    Spline Flags: WALKMODE, FLYING, CYCLIC, ENTER_CYCLE
    Movement Time: 36937
    Points Count: 4
    Point 0: 6713.656 1158.745 357.573
    Point 1: 6627.521 1346.398 365.9617
    Point 2: 6532.312 1322.547 391.823
    Point 3: 6511.499 1175.65 393.3785

    Path Lenght 615.58657483662
    Speed  16.66585
    */

    void PacketBuilder::WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << uint32(count + 1);
        data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.getPoint(1), count);
    }

    void PacketBuilder::SplinePathSend(const UnitMovement& mov, MsgDeliverer& broadcast)
    {
        mov_assert(mov.SplineEnabled() && mov.move_spline.Initialized());

        WorldPacket data(MSG_NULL_ACTION, 64);
        WriteCommonMonsterMovePart(mov, data);

        const MoveSpline& move_spline = mov.move_spline;
        const Spline<int32>& spline = move_spline.spline;
        MoveSplineFlag splineflags = move_spline.splineflags;
        if (splineflags & MoveSplineFlag::Mask_CatmullRom)
        {
            if (splineflags.cyclic)
                WriteCatmullRomCyclicPath(spline, data);
            else
                WriteCatmullRomPath(spline, data);
        } 
        else
            WriteLinearPath(spline, data);

        broadcast(data);
    }

    void PacketBuilder::Client_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket& data)
    {
        if (uint16 opcode = C_Mode2Opc_table[mode][mov.HasMode(mode)])
        {
            data.SetOpcode(opcode);
            data << mov.Owner.GetPackGUID();
            data << uint32(0);  // sequence Id
        }
    }

    void PacketBuilder::Client_SpeedUpdate(const UnitMovement& mov, SpeedType ty, WorldPacket& data)
    {
        uint16 opcode = C_Speed2Opc_table[ty][2];

        data.SetOpcode(opcode);
        data << mov.Owner.GetPackGUID();
        WriteClientStatus(mov,data);
        data << mov.GetSpeed(ty);
    }

    void PacketBuilder::FullUpdate(const UnitMovement& mov, ByteBuffer& data)
    {
        WriteClientStatus(mov,data);

        data.append<float>(&mov.m_float_values[SpeedWalk], SpeedMaxCount);

        if (mov.SplineEnabled())
        {
            const MoveSpline& move_spline = mov.move_spline;
            MoveSplineFlag splineFlags = move_spline.splineflags;

            data << splineFlags.raw;

            if (splineFlags.final_angle)
            {
                data << move_spline.facing.angle;
            }
            else if (splineFlags.final_target)
            {
                data << move_spline.facing.target;
            }
            else if(splineFlags.final_point)
            {
                data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            }

            data << move_spline.timePassed();
            data << move_spline.Duration();
            data << move_spline.GetId();

            data << float(1.f);//splineInfo.duration_mod;
            data << float(1.f);//splineInfo.duration_mod_next;

            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;

            uint32 nodes = move_spline.getPath().size();
            data << nodes;
            data.append<Vector3>(&move_spline.getPath()[0], nodes);
            data << uint8(move_spline.spline.mode());
            data << (move_spline.isCyclic() ? Vector3::zero() : move_spline.FinalDestination());
        }
    }

    void PacketBuilder::FullUpdate(const Transportable& mov, ByteBuffer& data)
    {
        // TODO: find a better way for udate flag manipulation
        //size_t wpos = data.wpos();
        //data.put<uint16>(wpos, data.read<uint16>(wpos) | UPDATEFLAG_POSITION);

        if (mov.IsBoarded())
            data << mov.GetTransport()->Owner.GetPackGUID();
        else
            data << uint8(0);
        
        data << mov.GetLocalPosition();
        data << mov.GetGlobalPosition();
        //if(GetTypeId() == TYPEID_CORPSE)
            //*data << float(((WorldObjectType)this)->GetOrientation());
        //else
            data << float(0);
    }

    void PacketBuilder::FullUpdate(const MovementBase& mov, ByteBuffer& data)
    {
        data << mov.GetGlobalPosition();
    }

    void PacketBuilder::ReadClientStatus(ClientMoveState& mov, ByteBuffer& data)
    {
        data >> mov.moveFlags.raw;
        data >> mov.moveFlags2.raw;

        data >> mov.ms_time;
        data >> mov.world_position;

        if (mov.moveFlags.ontransport)
        {
            mov.t_guid = data.readPackGUID();
            data >> mov.transport_position;
            data >> mov.transport_time;
            data >> mov.transport_seat;

            if (mov.moveFlags2.interp_move)
                data >> mov.transport_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || mov.moveFlags2.allow_pitching)
        {
            data >> mov.pitch;
        }

        data >> mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data >> mov.jump_velocity;
            data >> mov.jump_sinAngle;
            data >> mov.jump_cosAngle;
            data >> mov.jump_xy_velocy;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data >> mov.spline_elevation;
        }
    }

    void PacketBuilder::WriteClientStatus(const UnitMovement& mov, ByteBuffer& data)
    {
        const _ClientMoveState& un = mov.m_unused;

        data << mov.moveFlags.raw;
        data << un.moveFlags2.raw;

        data << mov.GetUpdater().TickTime(); // or mov.getLastUpdate() ?
        data << mov.GetGlobalPosition();

        if (mov.moveFlags.ontransport)
        {
            data << mov.GetTransport()->Owner.GetPackGUID();
            data << mov.GetLocalPosition();
            data << un.transport_time;
            data << un.transport_seat;

            if (un.moveFlags2.interp_move)
                data << un.transport_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || un.moveFlags2.allow_pitching)
        {
            data << un.pitch;
        }

        data << un.fallTime;

        if (mov.moveFlags.falling)
        {
            data << un.jump_velocity;
            data << un.jump_sinAngle;
            data << un.jump_cosAngle;
            data << un.jump_xy_velocy;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data << un.spline_elevation;
        }
    }

    void PacketBuilder::WriteClientStatus(const ClientMoveState& mov, ByteBuffer& data)
    {
        data << mov.moveFlags.raw;
        data << mov.moveFlags2.raw;

        data << mov.ms_time;
        data << mov.world_position;

        if (mov.moveFlags.ontransport)
        {
            data.appendPackGUID(mov.t_guid);
            data << mov.transport_position;
            data << mov.transport_time;
            data << mov.transport_seat;

            if (mov.moveFlags2.interp_move)
                data << mov.transport_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || mov.moveFlags2.allow_pitching)
        {
            data << mov.pitch;
        }

        data << mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data << mov.jump_velocity;
            data << mov.jump_sinAngle;
            data << mov.jump_cosAngle;
            data << mov.jump_xy_velocy;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data << mov.spline_elevation;
        }
    }

    void PacketBuilder::SplineSyncSend(const UnitMovement& mov, MsgDeliverer& broadcast)
    {
        mov_assert(mov.SplineEnabled());
        const MoveSpline& move_spline = mov.move_spline;

        WorldPacket data(SMSG_FLIGHT_SPLINE_SYNC, 13);
        data << (float)(move_spline.timePassed() / (float)move_spline.Duration());
        data << mov.Owner.GetPackGUID();
        broadcast(data);
    }

    void PacketBuilder::Send_MSG_MOVE_TELEPORT(const UnitMovement& mov, MsgDeliverer& broadcast)
    {
        WorldPacket data(MSG_MOVE_TELEPORT, 64);
        data << mov.Owner.GetPackGUID();
        WriteClientStatus(mov, data);
        broadcast(data);
    }

    void PacketBuilder::Send_HeartBeat(const UnitMovement& mov, MsgDeliverer& broadcast)
    {
        WorldPacket data(MSG_MOVE_HEARTBEAT, 64);
        data << mov.Owner.GetPackGUID();
        WriteClientStatus(mov, data);
        broadcast(data);
    }
}
