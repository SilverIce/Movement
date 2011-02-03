
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

    struct MsgDeliverMethtod
    {
        virtual void operator()(WorldPacket&){}
    };

    class MovementState;

    class PacketBuilder
    {
        MovementState& mov;
        MovControlType mode;

        void Client_SpeedUpdate(SpeedType type, WorldPacket&) const;
        void Client_MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void Client_PathUpdate(WorldPacket&) const;

        void Spline_SpeedUpdate(SpeedType type, WorldPacket&) const;
        void Spline_MoveModeUpdate(MoveMode mode, WorldPacket&) const;
        void Spline_PathUpdate(WorldPacket&) const;

        // helpers
        void WriteClientStatus(ByteBuffer&) const;

    public:
        PacketBuilder(MovementState& dat, MovControlType c);
        ~PacketBuilder();

        void SpeedUpdate(SpeedType type, MsgDeliverMethtod&) const;
        void MoveModeUpdate(MoveMode mode, MsgDeliverMethtod&) const;
        void PathUpdate(MsgDeliverMethtod&) const;
        void FullUpdate(ByteBuffer& ) const;

        void SetControl(MovControlType c) { mode = c; }
        MovControlType GetControl() const { return mode; }
    };
}
