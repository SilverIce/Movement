
#include "opcodes.h"

#include "UnitMovement.h"
#include "packet_builder.h"
#include "MoveSpline.h"
#include "ByteBufferExtensions.h"
#include "Object.h"
#include "moveupdater.h"
#include "ClientMoveStatus.h"

namespace Movement
{
    const uint16 S_Speed2Opc_table[SpeedMaxCount]=
    {
        {/*MOVE_WALK,*/         SMSG_SPLINE_SET_WALK_SPEED},
        {/*MOVE_RUN,*/          SMSG_SPLINE_SET_RUN_SPEED},
        {/*MOVE_SWIM_BACK,*/    SMSG_SPLINE_SET_SWIM_BACK_SPEED},
        {/*MOVE_SWIM,*/         SMSG_SPLINE_SET_SWIM_SPEED},
        {/*MOVE_RUN_BACK,*/     SMSG_SPLINE_SET_RUN_BACK_SPEED},
        {/*MOVE_FLIGHT,*/       SMSG_SPLINE_SET_FLIGHT_SPEED},
        {/*MOVE_FLIGHT_BACK,*/  SMSG_SPLINE_SET_FLIGHT_BACK_SPEED},
        {/*MOVE_TURN_RATE,*/    SMSG_SPLINE_SET_TURN_RATE},
        {/*MOVE_PITCH_RATE,*/   SMSG_SPLINE_SET_PITCH_RATE},
    };

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
    };

    const uint16 C_Mode2Opc_table[MoveModeMaxCount][2]=
    {
        {/*WALK_BEGAN,*/        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/*ROOT_BEGAN,*/        SMSG_FORCE_MOVE_UNROOT,        SMSG_FORCE_MOVE_ROOT},
        {/*SWIM_BEGAN,*/        MSG_NULL_ACTION,               MSG_NULL_ACTION},
        {/*WATERWALK_MODE,*/    SMSG_MOVE_LAND_WALK,           SMSG_MOVE_WATER_WALK},
        {/*SLOW_FALL_BEGAN,*/   SMSG_MOVE_NORMAL_FALL,         SMSG_MOVE_FEATHER_FALL},
        {/*HOVER_BEGAN,*/       SMSG_MOVE_UNSET_HOVER,         SMSG_MOVE_SET_HOVER},
        {/*FLY_BEGAN,*/         SMSG_MOVE_UNSET_CAN_FLY,       SMSG_MOVE_SET_CAN_FLY},
        {/*levitation mode*/    MSG_NULL_ACTION,               MSG_NULL_ACTION},    // no opcodes
    };

    const uint16 SetSpeed2Opc_table[SpeedMaxCount][2]=
    {
        {MSG_MOVE_SET_WALK_SPEED,       SMSG_FORCE_WALK_SPEED_CHANGE},
        {MSG_MOVE_SET_RUN_SPEED,        SMSG_FORCE_RUN_SPEED_CHANGE},
        {MSG_MOVE_SET_SWIM_BACK_SPEED,  SMSG_FORCE_SWIM_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_SWIM_SPEED,       SMSG_FORCE_SWIM_SPEED_CHANGE},
        {MSG_MOVE_SET_RUN_BACK_SPEED,   SMSG_FORCE_RUN_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_FLIGHT_SPEED,     SMSG_FORCE_FLIGHT_SPEED_CHANGE},
        {MSG_MOVE_SET_FLIGHT_BACK_SPEED,SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_TURN_RATE,        SMSG_FORCE_TURN_RATE_CHANGE},
        {MSG_MOVE_SET_PITCH_RATE,       SMSG_FORCE_PITCH_RATE_CHANGE},
    };

    typedef void (*SpeedPtr)(const UnitMovement&,SpeedType,WorldPacket&);
    typedef void (*MoveModePtr)(const UnitMovement&,MoveMode,WorldPacket&);
    typedef void (*PathPtr)(const UnitMovement&,WorldPacket&);

    void PacketBuilder::SpeedUpdate(const UnitMovement& mov, SpeedType type, MsgDeliverMethtod& broadcast)
    {
        static const SpeedPtr speed_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_SpeedUpdate,
            &PacketBuilder::Spline_SpeedUpdate,
        };

        WorldPacket data;
        speed_ptrs[mov.GetControl()](mov, type, data);

        if (!data.empty())  // currently it can be empty
            broadcast(data);
    }

    void PacketBuilder::MoveModeUpdate(const UnitMovement& mov, MoveMode move_mode, MsgDeliverMethtod& broadcast)
    {
        static const MoveModePtr move_mode_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_MoveModeUpdate,
            &PacketBuilder::Spline_MoveModeUpdate,
        };

        WorldPacket data;
        move_mode_ptrs[mov.GetControl()](mov, move_mode, data);

        if (!data.empty())
            broadcast(data);
    }

    void PacketBuilder::SplinePathSend(const UnitMovement& mov, MsgDeliverMethtod& broadcast)
    {
        WorldPacket data(MSG_NULL_ACTION, 64);
        Spline_PathSend(mov, data);

        if (data.GetOpcode() != MSG_NULL_ACTION)
            broadcast(data);
    }

    void PacketBuilder::Spline_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket& data)
    {
        uint16 opcode = S_Speed2Opc_table[type];

        data.Initialize(opcode, 8+4);
        data << mov.Owner.GetPackGUID();
        data << mov.GetSpeed(type);
    }

    void PacketBuilder::Spline_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket& data)
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];

        data.Initialize(opcode, 8);
        data << mov.Owner.GetPackGUID();
    }

    void PacketBuilder::Spline_PathSend(const UnitMovement& mov, WorldPacket& data)
    {
        mov_assert(mov.SplineEnabled() && mov.move_spline.Initialized());
        uint16 opcode = SMSG_MONSTER_MOVE;


        const MoveSpline& move_spline = mov.move_spline;
        const MoveSpline::MySpline& spline = move_spline.spline;
        data.SetOpcode(opcode);

        // TODO: find more generic way
        if (!mov.SplineEnabled())
        {
            data << mov.Owner.GetPackGUID();
            data << uint8(0);
            data << mov.GetPosition3();
            data << move_spline.GetId();
            data << uint8(MonsterMoveStop);
            return;
        }

        const Vector3 * real_path = &spline.getPoint(spline.first());
        uint32 last_idx = spline.pointsCount() - 1;

        data << mov.Owner.GetPackGUID();
        data << uint8(0);
        data << real_path[0];
        data << move_spline.GetId();

        MoveSplineFlag splineflags = move_spline.splineflags;

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

        data << uint32(last_idx);

        if (splineflags & MoveSplineFlag::Mask_CatmullRom)
        {
            data.append<Vector3>(&real_path[1], last_idx);
        }
        else
        {
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
    }

    void PacketBuilder::Client_MoveModeUpdate(const UnitMovement& mov, MoveMode /*type*/, WorldPacket& data)
    {
        WriteClientStatus(mov, data);
    }

    void PacketBuilder::Client_SpeedUpdate(const UnitMovement& mov, SpeedType ty, WorldPacket& data)
    {
        bool forced = false;

        uint16 opcode = SetSpeed2Opc_table[ty][forced];

        data.Initialize(opcode, 30);
        data << mov.Owner.GetPackGUID();

        if(!forced)
        {
            WriteClientStatus(mov,data);
        }
        else
        {
            data << (uint32)0;                                  // moveEvent, NUM_PMOVE_EVTS = 0x39
            if (ty == SpeedRun)
                data << uint8(0);                               // new 2.1.0
        }

        data << mov.GetSpeed(ty);
    }

    void PacketBuilder::FullUpdate(const UnitMovement& mov, ByteBuffer& data)
    {
        WriteClientStatus(mov,data);

        data.append<float>(&mov.speed[SpeedWalk], SpeedMaxCount);

        if (mov.SplineEnabled())
        {
            const MoveSpline& move_spline = mov.move_spline;
            MoveSplineFlag splineFlags = mov.move_spline.splineflags;

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

            data << move_spline.finalDestination;
        }
    }

    void PacketBuilder::ReadClientStatus(ClientMoveState& mov, ByteBuffer& data)
    {
        data >> mov.moveFlags.raw;
        data >> mov.moveFlags2.raw;

        data.read_skip<uint32>();// >> mov.last_update_time;
        data >> mov.position3;
        data >> mov.orientation;

        if (mov.moveFlags.ontransport)
        {
            data >> mov.transport.t_guid;
            data >> mov.transport.position;
            data >> mov.transport.orientation;
            data >> mov.transport.t_time;
            data >> mov.transport.t_seat;

            if (mov.moveFlags2.interp_move)
                data >> mov.transport.t_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || mov.moveFlags2.allow_pitching)
        {
            data >> mov.pitch;
        }

        data >> mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data >> mov.j_velocity;
            data >> mov.j_sinAngle;
            data >> mov.j_cosAngle;
            data >> mov.j_xy_velocy;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data >> mov.spline_elevation;
        }
    }

    void PacketBuilder::WriteClientStatus(const UnitMovement& mov, ByteBuffer& data)
    {
        data << mov.moveFlags.raw;
        data << mov.moveFlags2.raw;

        data << mov.last_update_time;
        data << mov.position;

        if (mov.moveFlags.ontransport)
        {
            data.appendPackGUID(mov.m_transportInfo.t_guid);
            data << mov.transport_offset;
            data << mov.m_transportInfo.t_time;
            data << mov.m_transportInfo.t_seat;

            if (mov.moveFlags2.interp_move)
                data << mov.m_transportInfo.t_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying) || mov.moveFlags2.allow_pitching)
        {
            data << mov.pitch;
        }

        data << mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data << mov.j_velocity;
            data << mov.j_sinAngle;
            data << mov.j_cosAngle;
            data << mov.j_xy_velocy;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data << mov.spline_elevation;
        }
    }
}
