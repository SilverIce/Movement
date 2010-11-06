
#pragma once

#include "mov_constants.h"

class ByteBuffer;
class WorldPacket;

namespace Movement
{
    enum MovControlType
    {
        MovControlClient,
        MovControlServer,
        MovControlCount,
    };

    class MovementState;

    class PacketBuilder
    {
        const MovementState& mov;
        MovControlType mode;

        void Client_SpeedUpdate(SpeedType type, WorldPacket&) const;
        void Client_MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void Client_PathUpdate(WorldPacket&) const;

        void Spline_SpeedUpdate(SpeedType type, WorldPacket&) const;
        void Spline_MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void Spline_PathUpdate(WorldPacket&) const;

    public:
        PacketBuilder(MovementState *const dat, MovControlType c);
        ~PacketBuilder();

        void SpeedUpdate(SpeedType type, WorldPacket& ) const;
        void MoveModeUpdate(MoveMode mode, WorldPacket& ) const;
        void PathUpdate(WorldPacket& ) const;
        void FullUpdate(ByteBuffer& ) const;

        void SetControl(MovControlType c) { mode = c; }
        MovControlType GetControl() const { return mode; }
    };
}
