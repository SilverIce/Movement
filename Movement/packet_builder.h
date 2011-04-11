
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
    struct MsgDeliverer
    {
        virtual void operator()(WorldPacket&) = 0;
    };

    class UnitMovement;
    struct ClientMoveState;
    template<class T> class Spline;

    class PacketBuilder
    {
        static void Client_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket&);
        static void Client_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket&);

        static void Spline_SpeedUpdate(const UnitMovement& mov, SpeedType type, WorldPacket&);
        static void Spline_MoveModeUpdate(const UnitMovement& mov, MoveMode mode, WorldPacket&);
        static void Spline_PathSend(const UnitMovement& mov, WorldPacket&);

        static void WriteCommonMonsterMovePart(const UnitMovement& mov, WorldPacket& data);
        static void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data);
        static void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data);
        static void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data);

    public:

        static void SpeedUpdate(const UnitMovement& mov, SpeedType type, MsgDeliverer&);
        static void MoveModeUpdate(const UnitMovement& mov, MoveMode mode, MsgDeliverer&);
        static void SplinePathSend(const UnitMovement& mov, MsgDeliverer&);
        static void SplineSyncSend(const UnitMovement& mov, MsgDeliverer&);
        static void FullUpdate(const UnitMovement& mov, ByteBuffer& );

        static void WriteClientStatus(const UnitMovement& mov, ByteBuffer& data);
        static void WriteClientStatus(const ClientMoveState& mov, ByteBuffer& data);
        static void ReadClientStatus(ClientMoveState& state, ByteBuffer& data);
    };
}
