#include "Imports.h"
#include <windows.h>

namespace Movement { namespace Imports
{
    void UpdateMapPosition(WorldObject*, const Movement::Location&) {}
    void BroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg) {}
    void BroadcastMessage(WorldObject const* obj, WorldPacket& msg) {}
    void SendPacket(void * socket, const WorldPacket& data) {}
    unsigned int getMSTime() { return GetTickCount();}
}}
