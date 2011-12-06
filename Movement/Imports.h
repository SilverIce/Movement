#pragma once

class WorldObject;
class WorldPacket;

namespace Movement{
    struct Location;
    class MovementMessage;
}

namespace Movement { namespace Imports
{
    extern void BroadcastMoveMessage(WorldObject const* obj, Movement::MovementMessage& msg);
    extern void UpdateMapPosition(WorldObject*, const Movement::Location&);
    extern void BroadcastMessage(WorldObject const* obj, WorldPacket& msg);
    extern void SendPacket(void * socket, const WorldPacket& data);
    extern unsigned int getMSTime();
}}
