
/**
  file:         packet_builder.h
  author:       SilverIce
  created:      16:2:2011
*/

#pragma once

#include "mov_constants.h"

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    struct MsgDeliverMethtod
    {
        virtual void operator()(WorldPacket&) = 0;
    };

    class UnitMovement;
    struct ClientMoveState;

    class PacketBuilder
    {
        static void Client_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket&);
        static void Client_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket&);

        static void Spline_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket&);
        static void Spline_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket&);
        static void Spline_PathSend(const UnitMovement& mov, WorldPacket&);

    public:

        static void SpeedUpdate(const UnitMovement& mov, SpeedType type, MsgDeliverMethtod&);
        static void MoveModeUpdate(const UnitMovement& mov, MoveMode mode, MsgDeliverMethtod&);
        static void SplinePathSend(const UnitMovement& mov, MsgDeliverMethtod&);
        static void SplineSyncSend(const UnitMovement& mov, MsgDeliverMethtod&);
        static void FullUpdate(const UnitMovement& mov, ByteBuffer& );

        static void WriteClientStatus(const UnitMovement& mov, ByteBuffer& data);
        static void WriteClientStatus(const ClientMoveState& mov, ByteBuffer& data);
        static void ReadClientStatus(ClientMoveState& state, ByteBuffer& data);
    };
}
