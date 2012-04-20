#pragma once

#include "framework/typedefs.h"

class WorldObject;
class WorldPacket;

template<class> class QVector;

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
        void (CDECL* OnPositionChanged)(WorldObject*, const Location&);
        void (CDECL* BroadcastMoveMessage)(WorldObject const* obj, MovementMessage& msg);
        void (CDECL* BroadcastMessage)(WorldObject const* obj, WorldPacket& msg);
        void (CDECL* SendPacket)(void * socket, const WorldPacket& data);
        uint32 (CDECL* getMSTime)();
        void (CDECL* GeneratePath) (WorldObject const* obj, const Vector3& fromVec, const Vector3& toVec, bool flightPath, QVector<G3D::Vector3>& path);
        UnitMovement* (CDECL* GetUnit) (void* context, uint64 guid);
        UnitMovement* (CDECL* GetUnit2) (WorldObject const* obj, uint64 guid);

        void (CDECL* SpawnMark)     (WorldObject* obj, const Vector3& position);
        void (CDECL* SetUIntValue)  (WorldObject* obj, uint16 fieldIdx, uint32 value);
        uint32 (CDECL* GetUIntValue)  (WorldObject* obj, uint16 fieldIdx);
    };

    extern _Imports Imports;

    EXPORT bool InitModule(const _Imports& ftable);
}
