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
        ~UnitMovement() {}
    public:

        static UnitMovement* create(WorldObject& owner, uint64 ownerGuid, MoveUpdater& updater);
        void dealloc();

        inline UnitMovementImpl& Impl() { return m;}
        inline const UnitMovementImpl& Impl() const { return m;}

        void CleanReferences();

        std::string ToString();


        /* Needed for monster movement only*/
        void BindOrientationTo(UnitMovement& target);
        void UnbindOrientation();

        /** It changes raw position only and doesn't synchronizes it with clients.
            It has been added as additional initialization step */
        void SetPosition(const Location& position);
        const Location& GetPosition();
        const Vector3& GetPosition3();
        Vector3 direction();

    public:
        // Used by server side controlled movement
        uint32 MoveSplineId();
        void SetListener(class IListener * l);

    public:
        bool HasMode(MoveMode mode);
        void ApplyMoveMode(MoveMode mode, bool apply);

        void Teleport(const Location& loc);

        bool IsWalking();
        bool IsMoving();
        bool IsTurning();
        bool IsFlying();
        bool IsFalling();
        bool IsFallingFar();

        void SetCollisionHeight(float value);
        float GetCollisionHeight();

        void SetSpeed(SpeedType speed, float value);
        float GetSpeed(SpeedType speed);
        float GetCurrentSpeed();

        uint32 dbg_flags;

        void WriteCreate(ByteBuffer& buf);
    };
}

