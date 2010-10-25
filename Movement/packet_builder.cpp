
#include "packet_builder.h"
#include "mov_constants.h"
#include "opcodes.h"
#include "OutLog.h"

#include "Movement.h"
#include "SplineState.h"
#include <assert.h>

#include "WorldPacket_fake.h"

namespace Movement
{
    extern const uint32 Mode2Flag_table[];
    extern const uint16 S_Speed2Opc_table[];
    extern const uint16 S_Mode2Opc_table[MoveModeMaxCount][2];
    extern const uint16 SetSpeed2Opc_table[][2];
    extern const float BaseSpeed[SpeedMaxCount];

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

    void PacketBuilder::StateUpdate(WorldPacket& p) const
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

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opcode, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
        data << (float)mov.GetSpeed(type);
    }

    void PacketBuilder::Spline_MoveModeUpdate(MoveMode mode, WorldPacket& data) const
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opc, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
    }

    void PacketBuilder::Spline_PathUpdate(WorldPacket& data) const
    {
        const SplineState& splineInfo = mov.spline;
        const G3D::Array<Vector3>& path = splineInfo.spline.points;

        assert(path.size());

        uint16 opcode = SMSG_MONSTER_MOVE;
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //WorldPacket &data = wow_object->PrepareSharedMessage( SMSG_MONSTER_MOVE, 30, true);
        //data.append(mov.wow_object->GetPackGUID());
        data << uint8(0);
        // so positon became useless, there no more position, current position - only node,
        // or i'm not correct and need really send _current position?
        const Vector3& start = mov.position;
        data << start;

        data << uint32(splineInfo.last_ms_time);

        uint32 nodes_count = path.size();
        uint32 splineflags = splineInfo.GetSplineFlags();  // spline flags are here? not sure...

        if(splineflags & SPLINE_MASK_FINAL_FACING)
        {
            if (splineflags & SPLINEFLAG_FINALTARGET)
            {
                data << uint8(SPLINETYPE_FACINGTARGET);
                data << splineInfo.facing_info.target;
            }
            else if(splineflags & SPLINETYPE_FACINGANGLE)
            {
                data << uint8(SPLINETYPE_FACINGANGLE);
                data << splineInfo.facing_info.angle;
            }
            else if(splineflags & SPLINEFLAG_FINALFACING)
            {
                data << uint8(SPLINETYPE_FACINGSPOT);
                data << splineInfo.facing_info.spot;
            }
            else
                assert(false);
        }
        else
            data << uint8(SPLINETYPE_NORMAL);

        data << uint32(splineflags & ~SPLINE_MASK_NO_MONSTER_MOVE);
        data << uint32(nodes_count);

        if (splineflags & SPLINEFLAG_UNKNOWN3)
        {
            data << uint8(0);
            data << uint32(0);
        }

        data << splineInfo.duration();

        if (splineflags & SPLINEFLAG_TRAJECTORY)
        {
            data << float(0);   //z speed
            data << uint32(0); // some time
        }

        if(splineflags & (SPLINEFLAG_BEZIER3 | SPLINEFLAG_CATMULLROM))
        {
            for(uint32 i = 0; i < nodes_count; ++i)
                data << path[i];
        }
        else
        {
            const Vector3 &dest = path[nodes_count-1];
            data << dest;   // destination

            if(nodes_count > 1)
            {
                Vector3 vec = (start + dest) / 2;

                for(uint32 i = 0; i < nodes_count - 1; ++i)// "nodes_count-1" because destination point already appended
                {
                    Vector3 temp = vec - path[i];
                    data.appendPackXYZ(temp.x, temp.y, temp.z);
                }
            }
        }
    }



    void PacketBuilder::Client_MoveModeUpdate(MoveMode /*type*/, WorldPacket& data) const
    {
        //mov.wow_object->BuildHeartBeatMsg(&mov.wow_object->PrepareSharedMessage());
    }

    void PacketBuilder::Client_SpeedUpdate(SpeedType ty, WorldPacket& data) const
    {
        bool forced = true;

        //WorldObject *m = mov.wow_object;
        uint16 opcode = SetSpeed2Opc_table[ty][forced];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //WorldPacket& data = m->PrepareSharedMessage(opcode, 10); 
        //data.append(m->GetPackGUID());

        //if(!forced)
        //{
        //    m->WriteMovementBlock(data);
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
        // do nothing
    }
}
