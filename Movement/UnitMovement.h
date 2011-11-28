#pragma once

#include "typedefs.h"
#include "Location.h"
#include "MoveSplineInit.h"

class WorldObject;
class ByteBuffer;

namespace Movement
{
    enum MoveMode
    {
        MoveModeWalk,
        MoveModeRoot,
        MoveModeSwim,
        MoveModeWaterwalk,
        MoveModeAllowSlowfall,
        MoveModeHover,
        MoveModeFly,
        MoveModeGravityDisabled,
        MoveModeAllowFly,
        MoveModeAllowSwimFlyTransition,
        MoveMode_End
    };

    enum SpeedType
    {
        SpeedWalk           = 0,
        SpeedRun            = 1,
        SpeedSwimBack       = 2,
        SpeedSwim           = 3,
        SpeedRunBack        = 4,
        SpeedFlight         = 5,
        SpeedFlightBack     = 6,
        SpeedTurn           = 7,
        SpeedPitch          = 8,
        Speed_End           = 9,
    };

    double computeFallTime(float path_length);
    double computeFallElevation2(float time_passed, float start_velocy);
    double computeFallElevation(float time_passed);

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
        // Used by server side controlled movement
        uint32 MoveSplineId() const;
        void SetListener(class IListener * l);

    public:
        bool HasMode(MoveMode m) const;
        void ApplyMoveMode(MoveMode mode, bool apply);

        void Teleport(const Location& loc);

        bool IsWalking() const;
        bool IsMoving() const;
        bool IsTurning() const;
        bool IsFlying() const;
        bool IsFalling() const;
        bool IsFallingFar() const;

        void SetCollisionHeight(float value);
        float GetCollisionHeight() const;

        void SetSpeed(SpeedType type, float value);
        float GetSpeed(SpeedType type) const;
        float GetCurrentSpeed() const;

        uint32 dbg_flags;

        void WriteCreate(ByteBuffer& buf) const;
    };
}

