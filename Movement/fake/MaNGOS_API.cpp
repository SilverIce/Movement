#include "MaNGOS_API.h"

namespace MaNGOS_API
{
    void UpdateMapPosition(WorldObject*, const Movement::Location&) {}
    void BroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg) {}
    void BroadcastMessage(WorldObject const* obj, WorldPacket& msg) {}
    void SendPacket(void * socket, const WorldPacket& data) {}
};
