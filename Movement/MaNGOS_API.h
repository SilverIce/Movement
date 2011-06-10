#pragma once

class WorldObject;
class WorldPacket;

namespace Movement{
    struct Location;
    class MovementMessage;
}

namespace MaNGOS_API
{
    extern void UpdateMapPosition(WorldObject*, const Movement::Location&);
    extern void BroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg);
    extern void BroadcastMessage(WorldObject const* obj, WorldPacket& msg);
    extern void SendPacket(HANDLE socket, const WorldPacket& data);
};
