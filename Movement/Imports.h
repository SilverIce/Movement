#pragma once

#include <vector>
#include "framework/typedefs.h"

class WorldObject;
class WorldPacket;

namespace Movement{
    struct Location;
    class MovementMessage;
    class UnitMovement;
}

namespace G3D {
    class Vector3;
}

namespace Movement
{
    struct _Imports 
    {
        typedef void (CDECL*T_OnPositionChanged) (WorldObject*, const Movement::Location&);
        typedef void (CDECL*T_BroadcastMoveMessage) (WorldObject const* obj, Movement::MovementMessage& msg);
        typedef void (CDECL*T_BroadcastMessage) (WorldObject const* obj, WorldPacket& msg);
        typedef void (CDECL*T_SendPacket) (void * socket, const WorldPacket& data);
        typedef unsigned int (CDECL*T_getMSTime) ();
        typedef void (CDECL*T_GeneratePath) (WorldObject const* obj, const G3D::Vector3& fromVec, const G3D::Vector3& toVec, bool flightPath, std::vector<G3D::Vector3>& path);
        typedef UnitMovement* (CDECL*T_GetUnit) (void* context, uint64 guid);
        typedef UnitMovement* (CDECL*T_GetUnit2) (WorldObject const* obj, uint64 guid);

        T_OnPositionChanged       OnPositionChanged;
        T_BroadcastMoveMessage    BroadcastMoveMessage;
        T_BroadcastMessage        BroadcastMessage;
        T_SendPacket              SendPacket;
        T_getMSTime               getMSTime;
        T_GeneratePath            GeneratePath;
        T_GetUnit                 GetUnit;
        T_GetUnit2                GetUnit2;
    };

    extern _Imports Imports;

    EXPORT bool InitModule(const _Imports& ftable);
}
