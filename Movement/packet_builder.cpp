
#include "packet_builder.h"
#include "opcodes.h"

#include "UnitMovement.h"

#include "ByteBufferExtensions.h"
#include "Object.h"

namespace Movement
{
    typedef void (*SpeedPtr)(const MovementState&,SpeedType,WorldPacket&);
    typedef void (*MoveModePtr)(const MovementState&,MoveMode,WorldPacket&);
    typedef void (*PathPtr)(const MovementState&,WorldPacket&);

    void PacketBuilder::SpeedUpdate(const MovementState& mov, SpeedType type, MsgDeliverMethtod& broadcast)
    {
        static const SpeedPtr speed_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_SpeedUpdate,
            &PacketBuilder::Spline_SpeedUpdate,
        };

        WorldPacket data;
        speed_ptrs[mov.GetControl()](mov, type, data);

        if (!data.empty())  // currently it's can be empty
            broadcast(data);
    }

    void PacketBuilder::MoveModeUpdate(const MovementState& mov, MoveMode move_mode, MsgDeliverMethtod& broadcast)
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

    void PacketBuilder::PathUpdate(const MovementState& mov, MsgDeliverMethtod& broadcast)
    {
        static const PathPtr path_update_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_PathUpdate,
            &PacketBuilder::Spline_PathUpdate,
        };

        WorldPacket data;
        path_update_ptrs[mov.GetControl()](mov, data);

        if (!data.empty())
            broadcast(data);
    }

    void PacketBuilder::Spline_SpeedUpdate(const MovementState& mov, SpeedType type, WorldPacket& data)
    {
        uint16 opcode = S_Speed2Opc_table[type];

        data.Initialize(opcode, 8+4);
        data << mov.GetOwner().GetPackGUID();
        data << mov.GetSpeed(type);
    }

    void PacketBuilder::Spline_MoveModeUpdate(const MovementState& mov, MoveMode mode, WorldPacket& data)
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];

        data.Initialize(opcode, 8);
        data << mov.GetOwner().GetPackGUID();
    }

    void PacketBuilder::Spline_PathUpdate(const MovementState& mov, WorldPacket& data) const
    {
        uint16 opcode = SMSG_MONSTER_MOVE;

        const MoveSpline& splineInfo = mov.move_spline;

        data.Initialize(opcode, 60);

        // TODO: find more generic way
        if (!mov.SplineEnabled())
        {
            data << mov.GetOwner().GetPackGUID();
            data << uint8(0);
            data << mov.GetPosition3();
            data << splineInfo.GetId();
            data << uint8(MonsterMoveStop);
            return;
        }

        const Spline& spline = splineInfo.spline;
        const Vector3 * real_path = &spline.getPoint(spline.first());
        uint32 last_idx = spline.pointsCount() - 1;

        data << mov.GetOwner().GetPackGUID();
        data << uint8(0);
        data << real_path[0];
        data << splineInfo.GetId();

        uint32 splineflags = splineInfo.GetSplineFlags();

        switch(splineflags & SPLINE_MASK_FINAL_FACING)
        {
        default:
            data << uint8(MonsterMoveNormal);
            break;
        case SPLINEFLAG_FINAL_TARGET:
            data << uint8(MonsterMoveFacingTarget);
            data << splineInfo.facing.target;
            break;
        case SPLINEFLAG_FINAL_ANGLE:
            data << uint8(MonsterMoveFacingAngle);
            data << splineInfo.facing.angle;
            break;
        case SPLINEFLAG_FINAL_POINT:
            data << uint8(MonsterMoveFacingSpot);
            data << splineInfo.facing.spot.x << splineInfo.facing.spot.y << splineInfo.facing.spot.z;
            break;
        }

        data << uint32(splineflags & ~SPLINE_MASK_NO_MONSTER_MOVE);

        if (splineflags & SPLINEFLAG_ANIMATION)
        {
            data << splineInfo.animation_type;
            data << splineInfo.spec_effect_time;
        }

        data << uint32(splineInfo.modifiedDuration());

        if (splineflags & SPLINEFLAG_TRAJECTORY)
        {
            data << splineInfo.vertical_acceleration;
            data << splineInfo.spec_effect_time;
        }

        data << uint32(last_idx);

        if (splineflags & (SPLINEFLAG_FLYING | SPLINEFLAG_CATMULLROM))
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

    void PacketBuilder::Client_MoveModeUpdate(const MovementState& mov, MoveMode /*type*/, WorldPacket& data)
    {
        WriteClientStatus(mov, data);
    }

    void PacketBuilder::Client_SpeedUpdate(const MovementState& mov, SpeedType ty, WorldPacket& data)
    {
        bool forced = false;

        uint16 opcode = SetSpeed2Opc_table[ty][forced];

        data.Initialize(opcode, 30);
        data << mov.GetOwner().GetPackGUID();

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

    void PacketBuilder::Client_PathUpdate(const MovementState& mov, WorldPacket& data)
    {
        //WriteClientStatus(data);
        // do nothing
    }

    void PacketBuilder::FullUpdate(const MovementState& mov, ByteBuffer& data)
    {
        WriteClientStatus(mov,data);

        data.append<float>(&mov.speed[SpeedWalk], SpeedMaxCount);

        if (mov.HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
        {
            // for debugging
            static float unkf1 = 1.f;
            static float unkf2 = 1.f;
            static float unkf3 = 0.f;
            static float dur_multiplier = 1.f;

            static uint32 addit_flags = 0;

            const MoveSpline& splineInfo = mov.move_spline;
            uint32 splineFlags = splineInfo.splineflags | addit_flags;

            data << splineFlags;

            if(splineFlags & SPLINEFLAG_FINAL_ANGLE)
            {
                data << splineInfo.facing.angle;
            }
            else
            {
                if(splineFlags & SPLINEFLAG_FINAL_TARGET)
                {
                    data << splineInfo.facing.target;
                }
                else
                {
                    if(splineFlags & SPLINEFLAG_FINAL_POINT)
                    {
                        data << splineInfo.facing.spot.x << splineInfo.facing.spot.y << splineInfo.facing.spot.z;
                    }
                }
            }

            data << uint32(splineInfo.time_passed);
            data << uint32(splineInfo.duration);
            data << splineInfo.GetId();

            data << float(1.f);//splineInfo.duration_mod;
            data << float(1.f);//splineInfo.duration_mod_next;

            data << splineInfo.vertical_acceleration;
            data << splineInfo.spec_effect_time;

            uint32 nodes = splineInfo.getPath().size();
            data << nodes;
            data.append<Vector3>(&splineInfo.getPath()[0], nodes);

            data << uint8(splineInfo.spline.mode());

            data << splineInfo.finalDestination;
        }
    }

    void PacketBuilder::ReadClientStatus(MovementState& mov, ByteBuffer& data)
    {
        data >> mov.moveFlags;
        data >> mov.move_flags2;

        data.read_skip<uint32>();// >> mov.last_ms_time;
        data >> mov.position;

        if (mov.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            data >> mov.m_transport.t_guid;
            data >> mov.transport_offset;
            data >> mov.m_transport.t_time;
            data >> mov.m_transport.t_seat;

            if (mov.move_flags2 & MOVEFLAG2_INTERP_MOVE)
                data >> mov.m_transport.t_time2;
        }

        if (mov.HasMovementFlag(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING) || (mov.move_flags2 & MOVEFLAG2_ALLOW_PITCHING))
        {
            data >> mov.s_pitch;
        }

        data >> mov.fallTime;

        if (mov.HasMovementFlag(MOVEFLAG_FALLING))
        {
            data >> mov.j_velocity;
            data >> mov.j_sinAngle;
            data >> mov.j_cosAngle;
            data >> mov.j_xy_velocy;
        }

        if (mov.HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        {
            data >> mov.u_unk1;
        }
    }

    void PacketBuilder::WriteClientStatus(const MovementState& mov, ByteBuffer& data)
    {
        data << mov.moveFlags;
        data << mov.move_flags2;

        data << sMoveUpdater.TickCount();
        data << mov.position;

        if (mov.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            data << mov.m_transport.t_guid;
            data << mov.transport_offset;
            data << mov.m_transport.t_time;
            data << mov.m_transport.t_seat;

            if (mov.move_flags2 & MOVEFLAG2_INTERP_MOVE)
                data << mov.m_transport.t_time2;
        }

        if (mov.HasMovementFlag(MOVEFLAG_SWIMMING | MOVEFLAG_FLYING) || (mov.move_flags2 & MOVEFLAG2_ALLOW_PITCHING))
        {
            data << mov.s_pitch;
        }

        data << mov.fallTime;

        if (mov.HasMovementFlag(MOVEFLAG_FALLING))
        {
            data << mov.j_velocity;
            data << mov.j_sinAngle;
            data << mov.j_cosAngle;
            data << mov.j_xy_velocy;
        }

        if (mov.HasMovementFlag(MOVEFLAG_SPLINE_ELEVATION))
        {
            data << mov.u_unk1;
        }
    }
}
