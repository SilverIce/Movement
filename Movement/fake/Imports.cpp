#include "Imports.h"
#include <windows.h>

namespace Movement
{
    namespace
    {
        void FOnPositionChanged(WorldObject*, const Movement::Location&) {}
        void FBroadcastMessage(WorldObject const* obj, Movement::MovementMessage& msg) {}
        void FBroadcastMessage(WorldObject const* obj, WorldPacket& msg) {}
        void FSendPacket(void * socket, const WorldPacket& data) {}
        unsigned int FgetMSTime() { return GetTickCount();}

        UnitMovement* FGetUnit(void* context, uint64 guid)
        {
            return NULL;
        }
    }

    _Imports Imports = {
        &FOnPositionChanged,
        &FBroadcastMessage,
        &FBroadcastMessage,
        &FSendPacket,
        &FgetMSTime,
        &FGetUnit,
    };

    void SetupImports(const _Imports& ftable)
    {
        Imports = ftable;
    }
}
