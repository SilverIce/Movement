
#include "packet_builder.h"
#include "opcodes.h"
#include "OutLog.h"

#include "Movement.h"
#include <assert.h>

#include "ByteBufferExtensions.h"
#include "Object.h"

namespace Movement
{
    PacketBuilder::PacketBuilder(MovementState *const dat, MovControlType c)
        : mode(c), mov(*dat)
    {
    }

    PacketBuilder::~PacketBuilder()
    {
    }

    typedef void (PacketBuilder::*SpeedPtr)(SpeedType,WorldPacket&) const;
    typedef void (PacketBuilder::*MoveModePtr)(MoveMode,WorldPacket&) const;
    typedef void (PacketBuilder::*PathPtr)(WorldPacket&) const;

    void PacketBuilder::SpeedUpdate(SpeedType type, WorldPacket& p) const
    {
        static const SpeedPtr speed_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_SpeedUpdate,
            &PacketBuilder::Spline_SpeedUpdate,
        };

        (this->*speed_ptrs[mode])(type, p);
    }

    void PacketBuilder::MoveModeUpdate(MoveMode move_mode, WorldPacket& p) const
    {
        static const MoveModePtr move_mode_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_MoveModeUpdate,
            &PacketBuilder::Spline_MoveModeUpdate,
        };

        (this->*move_mode_ptrs[mode])(move_mode, p);
    }

    void PacketBuilder::PathUpdate(WorldPacket& p) const
    {
        static const PathPtr path_update_ptrs[MovControlCount] =
        {
            &PacketBuilder::Client_PathUpdate,
            &PacketBuilder::Spline_PathUpdate,
        };

        (this->*path_update_ptrs[mode])(p);
    }


    void PacketBuilder::Spline_SpeedUpdate(SpeedType type, WorldPacket& data) const
    {
        uint16 opcode = S_Speed2Opc_table[type];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        data.Initialize(opcode, 8+4);
        data << mov.m_owner->GetPackGUID();
        data << mov.GetSpeed(type);
    }

    void PacketBuilder::Spline_MoveModeUpdate(MoveMode mode, WorldPacket& data) const
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        data.Initialize(opcode, 8);
        data << mov.m_owner->GetPackGUID();
    }

    void PacketBuilder::Spline_PathUpdate(WorldPacket& data) const
    {
        uint16 opcode = SMSG_MONSTER_MOVE;
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        const MoveSpline& splineInfo = mov.move_spline;
        const PointsArray& path = splineInfo.getPath();

        data.Initialize(opcode, 30);

        // TODO: find more generic way
        if (path.empty())
        {
            data << mov.m_owner->GetPackGUID();
            data << uint8(0);
            data << mov.GetPosition3();
            data << uint32(splineInfo.sequence_Id);
            data << uint8(MonsterMoveStop);
            return;
        }

        const Vector3 * real_path = &path[splineInfo.spline.first()];
        uint32 last_idx = splineInfo.spline.pointsCount() - 1;

        data << mov.m_owner->GetPackGUID();
        data << uint8(0);
        data << real_path[0];
        data << uint32(splineInfo.sequence_Id);

        uint32 splineflags = splineInfo.GetSplineFlags();

        if (splineflags & (SPLINE_MASK_FINAL_FACING | SPLINEFLAG_DONE))
        {
            if (splineflags & SPLINEFLAG_FINAL_TARGET)
            {
                data << uint8(MonsterMoveFacingTarget);
                data << splineInfo.facing_target;
            }
            else if(splineflags & SPLINEFLAG_FINAL_ANGLE)
            {
                data << uint8(MonsterMoveFacingAngle);
                data << splineInfo.facing_angle;
            }
            else if(splineflags & SPLINEFLAG_FINAL_POINT)
            {
                data << uint8(MonsterMoveFacingSpot);
                data << splineInfo.facing_spot.x << splineInfo.facing_spot.y << splineInfo.facing_spot.z;
            }
            else if (splineflags & SPLINEFLAG_DONE) // its assumption only
            {
                data << uint8(MonsterMoveStop);
                return;
            }
            else
                assert(false);
        }
        else
            data << uint8(MonsterMoveNormal);

        data << uint32(splineflags & ~SPLINE_MASK_NO_MONSTER_MOVE);

        if (splineflags & SPLINEFLAG_ANIMATION)
        {
            data << splineInfo.animationType;
            data << splineInfo.animationTime;
        }

        data << uint32(splineInfo.duration);

        if (splineflags & SPLINEFLAG_TRAJECTORY)
        {
            data << splineInfo.parabolic.z_acceleration;
            data << splineInfo.parabolic.time_shift;
        }

        data << uint32(last_idx);

        if (splineflags & (SPLINEFLAG_FLYING | SPLINEFLAG_CATMULLROM))
        {
            //for(uint32 i = 1; i <= last_idx; ++i)
                //data << real_path[i];
            data.append<Vector3>(&real_path[1], last_idx);
        }
        else
        {
            data << real_path[last_idx];   // destination

            if (last_idx > 1)
            {
                Vector3 middle = (path[0] + path[last_idx]) / 2.f;
                Vector3 temp;

                // first and last points already appended
                for(uint32 i = 1; i < last_idx; ++i)
                {
                    temp = middle - real_path[i];
                    data.appendPackXYZ(temp.x, temp.y, temp.z);
                }
            }
        }
    }

    void PacketBuilder::Client_MoveModeUpdate(MoveMode /*type*/, WorldPacket& data) const
    {
        //WriteClientStatus(data);
    }

    void PacketBuilder::Client_SpeedUpdate(SpeedType ty, WorldPacket& data) const
    {
        bool forced = true;

        //WorldObject *m = mov.wow_object;
        uint16 opcode = SetSpeed2Opc_table[ty][forced];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //data.Initialize(opcode, 10); 
        //data << mov.m_owner->GetPackGUID();

        //if(!forced)
        //{
        //    WriteClientStatus(data);
        //}
        //else
        //{
        //    if(m->GetTypeId() == TYPEID_PLAYER)
        //        ++((Player*)this)->m_forced_speed_changes[ty];
        //    data << (uint32)0;                                  // moveEvent, NUM_PMOVE_EVTS = 0x39
        //    if (ty == MOVE_RUN)
        //        data << uint8(0);                               // new 2.1.0
        //}

        //data << mov.GetSpeed(ty);
    }

    void PacketBuilder::Client_PathUpdate(WorldPacket& data) const
    {
        //WriteClientStatus(data);
        // do nothing
    }

    void PacketBuilder::WriteClientStatus( ByteBuffer& data ) const
    {
        data << mov.moveFlags;
        data << mov.move_flags2;

        data << mov.last_ms_time;
        data << mov.position;

        if (mov.HasMovementFlag(MOVEFLAG_ONTRANSPORT))
        {
            data << mov.m_transport.t_guid;
            data << mov.m_transport.t_offset;
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

    void PacketBuilder::FullUpdate( ByteBuffer& data) const
    {
        WriteClientStatus(data);

        //for (int i = SpeedWalk; i < SpeedMaxCount; ++i)
            //data << mov.GetSpeed((SpeedType)i);
        data.append<float>(&mov.speed[SpeedWalk], SpeedMaxCount - SpeedWalk);
        
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
                data << splineInfo.facing_angle;
            }
            else
            {
                if(splineFlags & SPLINEFLAG_FINAL_TARGET)
                {
                    data << splineInfo.facing_target;
                }
                else
                {
                    if(splineFlags & SPLINEFLAG_FINAL_POINT)
                    {
                        data << splineInfo.facing_spot.x << splineInfo.facing_spot.y << splineInfo.facing_spot.z;
                    }
                }
            }

            data << uint32(splineInfo.time_passed);
            data << uint32(splineInfo.duration);
            data << splineInfo.sequence_Id;

            data << splineInfo.duration_mod;            // duration mod?
            data << splineInfo.sync_coeff;              // sync coeff?

            data << splineInfo.parabolic.z_acceleration;// z_acceleration?
            data << splineInfo.parabolic.time_shift;	// parabolic time shift?

            uint32 nodes = splineInfo.getPath().size();
            data << nodes;
            /*for (uint32 i = 0; i < nodes; ++i)
            {
                data << splineInfo.getNode(i);
            }*/
            data.append<Vector3>(&splineInfo.getPath()[0], nodes);

            data << uint8(splineInfo.spline.mode());

            data << splineInfo.finalDestination;
        }
    }
}
