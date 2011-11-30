
/**
  file:         packet_builder.h
  author:       SilverIce
  created:      16:2:2011
*/

#pragma once

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    class UnitMovementImpl;
    struct ClientMoveState;
    template<class T> class Spline;

    class PacketBuilder
    {
        static void WriteCommonMonsterMovePart(const UnitMovementImpl& mov, WorldPacket& data);
        static void WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data);
        static void WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data);
        static void WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data);

    public:

        static void SplinePathSend(const UnitMovementImpl& mov);
        static void SplineSyncSend(const UnitMovementImpl& mov);
        static void FullUpdate(const UnitMovementImpl& mov, ByteBuffer& );

        static void WriteClientStatus(const ClientMoveState& mov, ByteBuffer& data);
        static void ReadClientStatus(ClientMoveState& state, ByteBuffer& data);
        static void Send_HeartBeat(const UnitMovementImpl& mov);     // actually i shouldn't use it: only client is author of such packets
    };
}
