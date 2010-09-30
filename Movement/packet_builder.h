
#pragma once

#include "mov_constants.h"

namespace Movement
{
    enum MovControlType
    {
        MovControlClient,
        MovControlServer,
        MovControlCount,
    };

    class WorldPacket;

    class PacketBuilder
    {
        class IPacketBuilderType * m_states[MovControlCount];
        MovControlType current;
    public:
        PacketBuilder(class MovementState *const dat, MovControlType c);
        ~PacketBuilder();

        void SpeedUpdate(UnitMoveType type, WorldPacket& ) const;
        void MoveModeUpdate(MoveMode mode, WorldPacket& ) const;
        void PathUpdate(WorldPacket& ) const;

        void SetControl(MovControlType c) { current = c; }
        MovControlType GetControl(MovControlType c) const { return current; }
    };
}
