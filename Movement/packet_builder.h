
#pragma once

#include "mov_constants.h"

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
        MovControlType current;
    public:
        PacketBuilder(MovementState *const dat, MovControlType c);
        ~PacketBuilder();

        void SpeedUpdate(SpeedType type, WorldPacket& ) const;
        void MoveModeUpdate(MoveMode mode, WorldPacket& ) const;
        void StateUpdate(WorldPacket& ) const;

        void SetControl(MovControlType c) { current = c; }
        MovControlType GetControl(MovControlType c) const { return current; }
    };
}
