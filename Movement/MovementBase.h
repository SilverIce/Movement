
/**
  file:         MovementBase.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/Component.h"
#include <QtCore/QTextStream>
#include "ObjectGuid.h"
#include "Location.h"
#include "Imports.h"
#include "MoveEnv.h"

class WorldObject;

namespace Movement
{
    struct MovingEntity_WOW : MovingEntity_Revolvable3
    {
    private:
        COMPONENT_TYPEID(MovingEntity_WOW);
        typedef MovingEntity_Revolvable3 base;

    public:

        void OnPositionChanged() {
            const Vector3& global = GlobalPosition();
            Imports.OnPositionChanged(Owner, Location(global.x,global.y,global.z,base::YawAngle()));
        }

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
            OnPositionChanged();
        }

        using base::YawAngle;

        void YawAngle(float value) {
            base::YawAngle(value);
            OnPositionChanged();
        }

        Vector4 GetGlobalLocation() const {
            return Vector4(GlobalPosition(),base::YawAngle());
        }

        void RelativeLocation(const Vector4& position) {
            base::YawAngle(position.w);
            base::RelativePosition(position.xyz());
            OnPositionChanged();
        }

        Vector4 RelativeLocation() const {
            return Vector4(base::RelativePosition(),base::YawAngle());
        }

        void toString(QTextStream& st) const override
        {
            st << endl << "guid 0x" << hex << Guid.GetRawValue() << dec;
            st << endl << "global   position " << GlobalPosition().toString().c_str();
            if (Environment()) {
                st << endl << "relative position " << RelativePosition().toString().c_str();
                st << endl << "attached to another moving entity";
            }
            if (uint32 attachedCount = BindedEntities().count())
                st << endl << "moving entities attached count " << attachedCount;
        }
    };
}
