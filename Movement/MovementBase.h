
/**
  file:         MovementBase.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"
#include "framework/Component.h"
#include "LinkedList.h"
#include "Location.h"
#include "ObjectGuid.h"

class WorldObject;

namespace Movement
{
    class MoveUpdater;
    class MovementBase;
    class Transportable;
    class IListener;
    class UnitMovementImpl;

    /** Makes local transport position <--> global world position conversions */
    struct CoordTranslator
    {
        static Vector3 ToGlobal(const Vector3& coord_sys, const Vector3& local_coord)
        {
            return (coord_sys + local_coord);
        }

        static Vector3 ToLocal(const Vector3& coord_sys, const Vector3& local_coord)
        {
            return (coord_sys - local_coord);
        }

        static Vector3 ToGlobal(const Vector3& coord_sys, const Vector2& direction, const Vector3& local_coord)
        {
            Vector3 result(coord_sys);
            result.x += local_coord.x*direction.y - local_coord.y*direction.x;
            result.y += local_coord.x*direction.x + local_coord.y*direction.y;
            result.z += local_coord.z;
            return result;
        }

        static Vector3 ToLocal(const Vector3& coord_sys, const Vector2& direction, const Vector3& global_coord)
        {
            Vector3 result;
            Vector3 diff(coord_sys - global_coord);
            result.x = diff.x*direction.y + diff.y*direction.x;
            result.y = diff.y*direction.y - diff.x*direction.x;
            result.z = diff.z;
            return result;
        }

        static Location ToGlobal(const Location& coord_sys, const Vector2& direction, const Location& local_coord)
        {
            Location result(ToGlobal(static_cast<const Vector3&>(coord_sys),direction,static_cast<const Vector3&>(local_coord)));
            // TODO: normalize orientation to be in range [0, 2pi)
            result.orientation = coord_sys.orientation + local_coord.orientation;
            return result;
        }

        static Location ToLocal(const Location& coord_sys, const Vector2& direction, const Location& global_coord)
        {
            Location result(ToLocal(static_cast<const Vector3&>(coord_sys),direction,static_cast<const Vector3&>(global_coord)));
            // TODO: normalize orientation to be in range [0, 2pi)
            result.orientation = coord_sys.orientation - global_coord.orientation;
            return result;
        }

        static Location ToGlobal(const Location& coord_sys, const Location& local_coord)
        {
            Vector2 direction(cos(coord_sys.orientation), sin(coord_sys.orientation));
            return ToGlobal(coord_sys,direction,local_coord);
        }

        static Location ToLocal(const Location& coord_sys, const Location& global_coord)
        {
            Vector2 direction(cos(coord_sys.orientation), sin(coord_sys.orientation));
            return ToLocal(coord_sys,direction,global_coord);
        }
    };

    typedef WorldObject& WorldObjectType;

    struct WowObject : ComponentT<WowObject>
    {
        WorldObject* object;
        ObjectGuid guid;
    };

    class MovementBase
    {
    public:

        explicit MovementBase(WorldObjectType owner) : Owner(owner)
        {
        }

        virtual ~MovementBase() {}
        virtual void CleanReferences() {}

        const Location& GetGlobalPosition() const { return world_position;}
        void SetGlobalPosition(const Location& loc);

        WorldObject& Owner;
        ObjectGuid Guid;

    protected:

        Location world_position;
        MovementBase(const MovementBase&);
        MovementBase& operator = (const MovementBase&);
    };
}
