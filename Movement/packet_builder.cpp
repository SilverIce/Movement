#include "packet_builder.h"
#include "Movement.h"
#include "OutLog.h"

#include "const_tables.h"
#include <assert.h>
#include "SplineState.h"

//#include "WorldPacket.h"

namespace Movement
{
    // temporary, fake implementation
    class WorldPacket
    {
    public:

        template<class T>
        WorldPacket& operator << (const T&)
        {
            return *this;
        }

        template<class T>
        void append(const T&)
        {
        }

        void appendPackXYZ(float,float,float)
        {
        }
    };

    class IPacketBuilderType
    {
    public:
        virtual void SpeedUpdate(UnitMoveType type, WorldPacket&) const = 0;
        virtual void MoveModeUpdate(MoveMode mode, WorldPacket&) const = 0;
        virtual void PathUpdate(WorldPacket&) const = 0;
    };

    class ClientBuilder : public IPacketBuilderType
    {
    public:
        MovementState const& mov;
        ClientBuilder(MovementState &dat) : mov(dat) {}

        void SpeedUpdate(UnitMoveType type, WorldPacket&) const;
        void MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void PathUpdate(WorldPacket& ) const;
    };

    class SplineBuilder : public IPacketBuilderType
    {
    public:
        MovementState const& mov;
        SplineBuilder(MovementState &dat) : mov(dat) {}

        void SpeedUpdate(UnitMoveType type, WorldPacket&) const;
        void MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void PathUpdate(WorldPacket&) const;
    };


    PacketBuilder::PacketBuilder(class MovementState *const dat, MovControlType c)
        : current(c)
    {
        m_states[MovControlClient] = new ClientBuilder(*dat);
        m_states[MovControlServer] = new SplineBuilder(*dat);
    }

    PacketBuilder::~PacketBuilder()
    {
        delete m_states[MovControlClient];
        delete m_states[MovControlServer];
    }

    void PacketBuilder::SpeedUpdate(UnitMoveType type, WorldPacket& p) const
    {
        m_states[current]->SpeedUpdate(type,p);
    }

    void PacketBuilder::MoveModeUpdate(MoveMode mode, WorldPacket& p) const
    {
        m_states[current]->MoveModeUpdate(mode,p);
    }

    void PacketBuilder::PathUpdate(WorldPacket& p) const
    {
        m_states[current]->PathUpdate(p);
    }




    void SplineBuilder::SpeedUpdate(UnitMoveType type, WorldPacket& data) const
    {
        uint16 opcode = S_Speed2Opc_table[type];
        sLog.write("PacketBuilder: control: Server, create x%X message", opcode);

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opcode, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
        data << (float)mov.GetSpeed(type);
    }

    void SplineBuilder::MoveModeUpdate(MoveMode mode, WorldPacket& data) const
    {
        uint16 opcode = S_Mode2Opc_table[ mode + (mov.HasMode(mode) ? 1 : 0) ];
        sLog.write("PacketBuilder: control: Server, create x%X message", opcode);

        //WorldPacket &data = mov.wow_object->PrepareSharedMessage(opc, 8+4);
        //data.append(mov.wow_object->GetPackGUID());
    }

    void SplineBuilder::PathUpdate(WorldPacket& data) const
    {
        const SplineState& spline = mov.spline;
        const NodeList& path = spline.spline_path;

        assert(path.size());

        uint16 opcode = SMSG_MONSTER_MOVE;
        sLog.write("PacketBuilder: control: Server, create x%X message", opcode);

        //WorldPacket &data = wow_object->PrepareSharedMessage( SMSG_MONSTER_MOVE, 30, true);
        //data.append(mov.wow_object->GetPackGUID());
        data << uint8(0);
        // so positon became useless, there no more position, current position - only node,
        // or i'm not correct and need really send _current position?
        //path[mov.node].WriteCoords3(data);
        const Vector3& start = mov.position;
        data << start;

        data << uint32(spline.time_stamp);

        uint32 nodes = path.size();
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
            data << uint8(0);

        data << uint32(splineflags);      // splineflags
        data << uint32(nodes);

        if (splineflags & SPLINEFLAG_UNKNOWN3)
        {
            data << uint8(0);
            data << uint32(0);
        }

        data << spline.move_time_full;

        if (splineflags & SPLINEFLAG_TRAJECTORY)
        {
            data << float(0);   //z speed
            data << uint32(0); // some time
        }

        if(splineflags & (SPLINEFLAG_FLYING | SPLINEFLAG_CATMULLROM))
        {
            for(uint32 i = 0; i < nodes; ++i)
                data.append(path[i].vec);
        }
        else
        {
            const Vector3 &dest = path.back().vec;
            data.append(dest);   // dest point

            if(nodes > 1)
            {
                Vector3 vec = (start + dest) / 2;

                for(uint32 i = 0; i < nodes - 1; ++i)// "nodes-1" because last node already appended
                {
                    Vector3 temp = vec - path[i].vec;
                    data.appendPackXYZ(temp.x, temp.y, temp.z);
                }
            }
        }
    }



    void ClientBuilder::MoveModeUpdate(MoveMode /*type*/, WorldPacket& /*apply*/) const
    {
        //mov.wow_object->BuildHeartBeatMsg(&mov.wow_object->PrepareSharedMessage());
    }

    void ClientBuilder::SpeedUpdate(UnitMoveType ty, WorldPacket& data) const
    {
        bool forced = true;

        //WorldObject *m = mov.wow_object;
        uint16 opcode = SetSpeed2Opc_table[ty][forced ? 1 : 0];
        sLog.write("PacketBuilder: control: Client, create x%X message", opcode);

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

    void ClientBuilder::PathUpdate(WorldPacket&) const
    {
        // do nothing
    }
}
