#pragma once

#include "typedefs.h"
#include "mov_constants.h"
#include "Location.h"
#include "MoveSplineInit.h"

class WorldObject;
class ByteBuffer;

namespace Movement
{
    class MoveUpdater;
    class UnitMovementImpl;

    class UnitMovement
    {
        UnitMovementImpl& m;
        UnitMovement(UnitMovementImpl& impl) : m(impl), dbg_flags(0) {}
        UnitMovement(const UnitMovement&);
        UnitMovement& operator = (const UnitMovement&);
    public:

        static UnitMovement* create(WorldObject& owner);
        ~UnitMovement();

        inline UnitMovementImpl& Impl() { return m;}
        inline const UnitMovementImpl& Impl() const { return m;}

        void CleanReferences();

        std::string ToString() const;

        void Initialize(const Location& position, MoveUpdater& updater);

        /* Needed for monster movement only*/
        void BindOrientationTo(UnitMovement& target);
        void UnbindOrientation();

        const Location& GetPosition() const;
        const Vector3& GetPosition3() const { return GetPosition();}
        Vector3 direction() const;

    public:
        void Unboard();
        void Board(UnitMovement& ps, const Location& local_pos, int8 seat);
        void UnboardAll();
        bool IsBoarded() const;

    public:
        // Used by server side controlled movement
        uint32 MoveSplineId() const;
        const Vector3& MoveSplineDest() const;
        int32 MoveSplineTimeElapsed() const;
        void SetListener(class IListener * l);

    public:
        bool HasMode(MoveMode m) const;

        /// Apply/remove modes
        void ApplyRootMode(bool apply);
        void ApplySwimMode(bool apply);
        void ApplyWalkMode(bool apply);
        void ApplyWaterWalkMode(bool apply);
        void ApplySlowFallMode(bool apply);
        void ApplyFlyMode(bool apply);
        void ApplyCanFlyMode(bool apply);
        void ApplyHoverMode(bool apply);
        void DisableGravity(bool apply);
        void ApplyCanSwimFlyTransitionMode(bool apply);

        void Teleport(const Location& loc);

        bool IsWalking() const;
        bool IsMoving() const;
        bool IsTurning() const;
        bool IsFlying() const;
        bool IsFalling() const;
        bool IsFallingFar() const;

        void SetCollisionHeight(float value);
        float GetCollisionHeight() const;

        void SetSpeed(SpeedType type, float s);
        float GetSpeed(SpeedType type) const;
        float GetCurrentSpeed() const;

        uint32 dbg_flags;

        void WriteCreate(ByteBuffer& buf) const;
    };
}

