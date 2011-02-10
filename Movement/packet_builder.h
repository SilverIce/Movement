
#pragma once

#include "mov_constants.h"

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    struct MsgDeliverMethtod
    {
        virtual void operator()(WorldPacket&){}
    };

    class MovementState;

    class PacketBuilder
    {
        static void Client_SpeedUpdate(const MovementState& mov, SpeedType type, WorldPacket&);
        static void Client_MoveModeUpdate(const MovementState& mov, MoveMode mode, WorldPacket&);
        static void Client_PathUpdate(const MovementState& mov, WorldPacket&);

        static void Spline_SpeedUpdate(const MovementState& mov, SpeedType type, WorldPacket&);
        static void Spline_MoveModeUpdate(const MovementState& mov, MoveMode mode, WorldPacket&);
        static void Spline_PathUpdate(const MovementState& mov, WorldPacket&);

    public:

        static void SpeedUpdate(const MovementState& mov, SpeedType type, MsgDeliverMethtod&);
        static void MoveModeUpdate(const MovementState& mov, MoveMode mode, MsgDeliverMethtod&);
        static void PathUpdate(const MovementState& mov, MsgDeliverMethtod&);
        static void FullUpdate(const MovementState& mov, ByteBuffer& );

        static void WriteClientStatus(const MovementState& mov, ByteBuffer& data);
        static void ReadClientStatus(MovementState& mov, ByteBuffer& data);
    };
}
