#pragma once

class WorldObject;
class WorldPacket;

namespace Movement{
    struct Location;
    class MovementMessage;
    class UnitMovement;
}

namespace Movement
{
    struct _Imports 
    {
        typedef void (*T_UpdateMapPosition) (WorldObject*, const Movement::Location&);
        typedef void (*T_BroadcastMoveMessage) (WorldObject const* obj, Movement::MovementMessage& msg);
        typedef void (*T_BroadcastMessage) (WorldObject const* obj, WorldPacket& msg);
        typedef void (*T_SendPacket) (void * socket, const WorldPacket& data);
        typedef unsigned int (*T_getMSTime) ();
        typedef UnitMovement* (*T_GetUnit) (void* context, uint64 guid);

        T_OnPositionChanged       OnPositionChanged;
        T_BroadcastMoveMessage    BroadcastMoveMessage;
        T_BroadcastMessage        BroadcastMessage;
        T_SendPacket              SendPacket;
        T_getMSTime               getMSTime;
        T_GetUnit                 GetUnit;
    };

    extern _Imports Imports;

    void SetupImports(const _Imports& ftable);
}
