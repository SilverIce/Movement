
/**
  file:         MovementBase.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/Component.h"
#include "ObjectGuid.h"
#include "Location.h"
#include "Imports.h"
#include "MoveEnv.h"

class WorldObject;

namespace Movement
{
    struct WowObject : ComponentT<WowObject>
    {
        ObjectGuid guid;
        WorldObject* object;

        explicit WowObject() : object(nullptr) {}
    };

    struct MovingEntity_WOW : MovingEntity_Revolvable2, HasTypeId<MovingEntity_WOW>
    {
    private:
        typedef MovingEntity_Revolvable2 base;

    public:

        ObjectGuid Guid;
        WorldObject* Owner;

        explicit MovingEntity_WOW() : Owner(nullptr) {}

        void Init(const ObjectGuid& guid, WorldObject* object) {
            Guid = guid;
            Owner = object;
        }

        using base::RelativePosition;

        void RelativePosition(const Vector3& position) {
            base::RelativePosition(position);
            Imports.OnPositionChanged(Owner, GetGlobalLocation());
        }

        using base::YawAngle;

        void YawAngle(float value) {
            base::YawAngle(value);
            Imports.OnPositionChanged(Owner, GetGlobalLocation());
        }

        Location GetGlobalLocation() const {
            return Location(GlobalPosition(),base::YawAngle());
        }

        void RelativeLocation(const Location& position) {
            base::YawAngle(position.orientation);
            base::RelativePosition(position);
            Imports.OnPositionChanged(Owner, GetGlobalLocation());
        }

        Location RelativeLocation() const {
            return Location(base::RelativePosition(),base::YawAngle());
        }
    };
}
