
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
#include "Context.h"

class WorldObject;

namespace Movement
{
    struct MovingEntity_WOW : Component, private MovingEntity_Revolvable3<MovingEntity_WOW>
    {
    private:
        COMPONENT_TYPEID(MovingEntity_WOW);
        typedef MovingEntity_Revolvable3<MovingEntity_WOW> base;

        // maybe not very good solution - just to allow MovingEntity_WOW access to his own fields:
        friend struct base;

        friend class Unit_Passenger;
        using base::SetEnvironment;

    public:
        using base::Environment;
        using base::YawAngle;
        using base::PitchAngle;
        using base::RollAngle;
        using base::SetRotationFromTangentLine;
        using base::RelativePosition;
        using base::GlobalPosition;
        using base::BindedEntities;

        const ObjectGuid Guid;
        WorldObject* const Owner;
        Context* const context;

        explicit MovingEntity_WOW() : Owner(nullptr), context(nullptr) {}

        void Init(const ObjectGuid& guid, WorldObject* object, Context& _context) {
            const_cast<ObjectGuid&>(Guid) = guid;
            const_cast<WorldObject*>(Owner) = object;
            const_cast<Context*>(context) = &_context;
            _context.registry.add(guid, *this);
        }

        void CleanReferences() {
            context->registry.remove(Guid);
            const_cast<Context*>(context) = nullptr;
        }

        ~MovingEntity_WOW() {
            assert_state(!context);
        }

        void OnPositionChanged() {
            const Vector3& global = GlobalPosition();
            Imports.OnPositionChanged(Owner, Location(global.x,global.y,global.z,YawAngle()));
        }

        Vector4 GetGlobalLocation() const {
            return Vector4(GlobalPosition(),base::YawAngle());
        }

        void RelativeLocation(const Vector4& position) {
            YawAngle(position.w);
            RelativePosition(position.xyz());
        }

        void RelativePosition(const Vector3& position) {
            base::RelativePosition(position);
            OnPositionChanged();
        }

        Vector4 RelativeLocation() const {
            return Vector4(RelativePosition(),YawAngle());
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

    /** For internal use only! Holds the info that helps unboard, delink passenger from the transport. */
    struct IPassenger : Component
    {
        virtual void Unboard() = 0;
        COMPONENT_TYPEID(IPassenger);
    };
}
