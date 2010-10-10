
#include "packet_builder.h"
#include "mov_constants.h"
#include "OutLog.h"

#include "Movement.h"
#include "SplineState.h"
#include <assert.h>

#include "WorldPacket_fake.h"

namespace Movement
{
    class IPacketBuilder
    {
    public:
        virtual void SpeedUpdate(MovementState const& mov, SpeedType type, WorldPacket&) const = 0;
        virtual void MoveModeUpdate(MovementState const& mov, MoveMode mode, WorldPacket&) const = 0;
        virtual void PathUpdate(MovementState const& mov, WorldPacket&) const = 0;
    };

    class ClientBuilder : public IPacketBuilder
    {
    public:
        void SpeedUpdate(MovementState const& mov, SpeedType type, WorldPacket&) const;
        void MoveModeUpdate(MovementState const& mov, MoveMode mode, WorldPacket&) const;
        void PathUpdate(MovementState const& mov, WorldPacket& ) const;
    };

    class SplineBuilder : public IPacketBuilder
    {
    public:
        void SpeedUpdate(MovementState const& mov, SpeedType type, WorldPacket&) const;
        void MoveModeUpdate(MovementState const& mov, MoveMode mode, WorldPacket&) const;
        void PathUpdate(MovementState const& mov, WorldPacket&) const;
    };

    static const IPacketBuilder * const m_states[MovControlCount] =
    {
        &(static const ClientBuilder()),
        &(static const SplineBuilder()),
    };


    PacketBuilder::PacketBuilder(class MovementState *const dat, MovControlType c)
        : current(c), mov(*dat)
    {
    }

    PacketBuilder::~PacketBuilder()
    {
    }

    void PacketBuilder::SpeedUpdate(SpeedType type, WorldPacket& p) const
    {
        m_states[current]->SpeedUpdate(mov,type,p);
    }

    void PacketBuilder::MoveModeUpdate(MoveMode mode, WorldPacket& p) const
    {
        m_states[current]->MoveModeUpdate(mov,mode,p);
    }

    void PacketBuilder::StateUpdate(WorldPacket& p) const
    {
        m_states[current]->PathUpdate(mov,p);
    }




    void SplineBuilder::SpeedUpdate(MovementState const& mov, SpeedType type, WorldPacket& data) const
    {
        uint16 opcode = S_Speed2Opc_table[type];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opcode, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
        data << (float)mov.GetSpeed(type);
    }

    void SplineBuilder::MoveModeUpdate(MovementState const& mov, MoveMode mode, WorldPacket& data) const
    {
        uint16 opcode = S_Mode2Opc_table[mode][mov.HasMode(mode)];
        sLog.write("PacketBuilder:  created %s message", OpcodeName(opcode));

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opc, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
    }

    void SplineBuilder::PathUpdate(MovementState const& mov, WorldPacket& data) const
    {
        const SplineState& spline = mov.spline;
        const std::vector<Vector3>& path = spline.points;

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

        data << uint32(spline.last_ms_time);

        uint32 nodes_count = path.size();
        uint32 splineflags = spline.GetSplineFlags();  // spline flags are here? not sure...

        if(splineflags & SPLINE_MASK_FINAL_FACING)
        {
            if (splineflags & SPLINEFLAG_FINALTARGET)
            {
                data << uint8(SPLINETYPE_FACINGTARGET);
                data << spline.facing_info.target;
            }
            else if(splineflags & SPLINETYPE_FACINGANGLE)
            {
                data << uint8(SPLINETYPE_FACINGANGLE);
                data << spline.facing_info.angle;
            }
            else if(splineflags & SPLINEFLAG_FINALFACING)
            {
                data << uint8(SPLINETYPE_FACINGSPOT);
                data << spline.facing_info.spot;
            }
            else
                assert(false);
        }
        else
            data << uint8(SPLINETYPE_NORMAL);

        data << uint32(splineflags);      // splineflags
        data << uint32(nodes_count);

        if (splineflags & SPLINEFLAG_UNKNOWN3)
        {
            data << uint8(0);
            data << uint32(0);
        }

        data << spline.duration;

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
            const Vector3 &dest = path.back();
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



    void ClientBuilder::MoveModeUpdate(MovementState const& mov, MoveMode /*type*/, WorldPacket& /*apply*/) const
    {
        //mov.wow_object->BuildHeartBeatMsg(&mov.wow_object->PrepareSharedMessage());
    }

    void ClientBuilder::SpeedUpdate(MovementState const& mov, SpeedType ty, WorldPacket& data) const
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

    void ClientBuilder::PathUpdate(MovementState const& mov, WorldPacket&) const
    {
        // do nothing
    }
}
