#pragma once

#include "framework/typedefs.h"
#include "framework/Component.h"
#include "Movement/MoveSplineInit.h"

class WorldObject;

class QByteArray;
class QString;

namespace Tasks {
    class ITaskExecutor;
}

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

    EXPORT double Gravity();
    EXPORT double computeFallTime(float path_length);
    EXPORT double computeFallElevation(float time_passed, float start_velocity);
    EXPORT double computeFallElevation(float time_passed);

    class UnitMovementImpl;
    class Location;

    class UnitPassenger;
    class Transporter;
    class Vehicle;
    class Context;

    class EXPORT UnitMovement : public Component
    {
        COMPONENT_TYPEID(UnitMovement);
        friend struct UnitMovementStruct;
        UnitMovementStruct* m;
        UnitMovement(const UnitMovement&);
        UnitMovement& operator = (const UnitMovement&);
    public:

        struct CreateInfo
        {
            WorldObject* object;
            Context& context;
            uint64 guid;
        };

        explicit UnitMovement(const CreateInfo&);
        ~UnitMovement();

        UnitMovementImpl& Impl();

        QString ToString();

        class Vehicle asVehicle();
        class UnitPassenger asPassenger();

        /* Needed for monster movement only*/
        void BindOrientationTo(UnitMovement& target);
        void UnbindOrientation();

        /** It changes raw position only and doesn't synchronizes it with clients.
            It has been added as additional initialization step */
        void SetPosition(const Location& position);
        Vector4 GetPosition();
        const Vector3& GetPosition3();
        /** Returns current moving direction. Returns Vector3(0,0,0) in case unit is not moving */
        Vector3 direction();

    public:
        // Used by server side controlled movement
        uint32 MoveSplineId();
        void SetListener(class IListener * l);

    public:
        bool HasMode(MoveMode mode);
        /** Enables or disables movement mode */
        void ApplyMoveMode(MoveMode mode, bool apply);

        void Teleport(const Location& loc);
        void Knockback(float directionAngle, float horizontalVelocity, float verticalVelocity);

        bool IsWalking();
        bool IsMoving();
        bool IsTurning();
        bool IsFlying();
        bool IsFalling();
        bool IsFallingFar();

        /** Modifies unit's collision box height */
        void SetCollisionHeight(float value);
        float GetCollisionHeight();

        void SetSpeed(SpeedType speed, float value);
        float GetSpeed(SpeedType speed);
        float GetCurrentSpeed();

        QByteArray WriteCreate();

        /** For testing */
        uint32 dbg_flags;
    };

    class EXPORT UnitPassenger
    {
        class Unit_Passenger * m;
    public:
        operator bool() const { return m != 0;}
        explicit UnitPassenger(Unit_Passenger* impl) : m(impl) {}

        int8 SeatId();
        /** Have no idea what else should i make publicly visible */
    };

    class VehicleImpl;
    class EXPORT Vehicle
    {
    private:
        friend class UnitMovement;
        VehicleImpl* m;
        explicit Vehicle(VehicleImpl* impl) : m(impl) {}
    public:
        operator bool() const { return m != 0;}

        static void Install(UnitMovement& transportOwner, uint32 vehicleId);

        void Board(int8 seatId, UnitMovement& passenger);
        UnitMovement* Passenger(int8 seatId);
        void UnBoard(int8 seatId);
        void UnboardAll();

        uint32 VehicleId();
    };
}
