
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
    class MovementBase;

    struct WowObject : ComponentT<WowObject>
    {
        WorldObject* object;
        ObjectGuid guid;
    };

    class MovementBase
    {
    public:

        explicit MovementBase(WorldObject& owner) : Owner(owner)
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
