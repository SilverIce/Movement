
#pragma once

#include "mov_constants.h"
#include "packet_builder.h"
#include "SplineState.h"

class WorldObject;

namespace Movement
{
    class MovementState
    {
        friend class PacketBuilder;

    public:
        MovementState(WorldObject * owner);

        ~MovementState()
        {
        }

        PacketBuilder& GetBuilder() { return msg_builder; }
        PacketBuilder msg_builder;

        void SetPosition(const Vector4& v, uint32 ms_time);
        void SetPosition(const Vector3& v, uint32 ms_time);

        const Vector4& GetPosition() const { return position;}
        const Vector3& GetPosition3() const { return (Vector3&)position;}

        WorldObject * m_owner;

        #pragma region field accessors

        //WorldObject* wow_object;

        /// Get-Set methtods

        /// Speed
        void SetSpeed(SpeedType type, float s) { speed[type] = s; }
        float GetSpeed(SpeedType type) const { return speed[type]; }
        float GetCurrentSpeed() const { return speed_obj.current; }

        /// Movement flags
        void AddMovementFlag(uint32 f) { moveFlags |= f; }
        void RemoveMovementFlag(uint32 f) { moveFlags &= ~f; }
        bool HasMovementFlag(uint32 f) const { return moveFlags & f; }
        uint32 GetMovementFlags() const { return moveFlags; }
        void SetMovementFlags(uint32 f) { moveFlags = f; }

        /// Direction flags
        void AddDirectionFlag(uint8 f) { direction_flags |= f; }
        void RemoveDirectionFlag(uint8 f) { direction_flags &= ~f; }
        bool HasDirectionFlag(uint8 f) const { return direction_flags & f; }
        uint8 GetDirectionFlags() const { return direction_flags; }
        void SetDirectionFlags(uint8 f) { direction_flags = f; }

        bool HasDest() const { return direction_flags & DIRECTIONS_MASK; }
        void ResetDirection()  { direction_flags = 0; }
        void SetForwardDirection() { direction_flags = DIRECTION_FORWARD; }

        bool SplineEnabled() const { return moveFlags & MOVEFLAG_SPLINE_ENABLED; }
        void EnableSpline() { moveFlags |= MOVEFLAG_SPLINE_ENABLED; }
        void DisableSpline() { moveFlags &= ~MOVEFLAG_SPLINE_ENABLED; }

        #pragma endregion

        /// Move Modes
        bool HasMode(MoveMode m) const { return move_mode & (1 << m);}
        void ApplyMoveMode(MoveMode mode, bool apply);
        /// end of Get-Set methtods

        /// Apply/remove modes
        void Root(bool apply) { ApplyMoveMode(MoveModeRoot, apply); }
        void Swim(bool apply) { ApplyMoveMode(MoveModeSwim, apply); }
        void Walk(bool apply) { ApplyMoveMode(MoveModeWalk, apply); }
        void WaterWalk(bool apply) { ApplyMoveMode(MoveModeWaterwalk, apply); }
        void SlowFall(bool apply) { ApplyMoveMode(MoveModeSlowfall, apply); }
        void Fly(bool apply) { ApplyMoveMode(MoveModeFly, apply); }
        void Hover(bool apply) { ApplyMoveMode(MoveModeHover, apply); }

        #pragma region fields


        /// Transport info
        struct TransportInfo
        {
            TransportInfo() : t_guid(0), t_time(0), t_seat(0), t_time2(2) {}
            uint64 t_guid;
            Vector4 t_offset;
            uint32 t_time;
            int8 t_seat;
            uint32 t_time2;
        };

        struct SpeedInfo
        {
            float current;
            float walk;
            float run;
            float run_back;
            float swim;
            float swim_back;
            float flight;
            float flight_back;
            float turn;
            float pitch;
        };

        // time-position pair
        Vector4         position;
        uint32          last_ms_time;

        uint32          move_mode;

        union {
            uint8       direction_flags;
            uint32      moveFlags;
        };
        uint16          move_flags2;

        // swimming and flying
        float           s_pitch;
        // last fall time
        uint32          fallTime;
        float           fallStartElevation;
        // jumping
        float           j_velocity, j_sinAngle, j_cosAngle, j_xy_velocy;

        float           u_unk1;

        TransportInfo   m_transport;
   
        union {
            SpeedInfo   speed_obj;
            float       speed[SpeedMaxCount];
        };

        MoveSpline      move_spline;

        #pragma endregion


        /// Some client's formulas:

        void ReCalculateCurrentSpeed();
        float CalculateCurrentSpeed(bool use_walk_forced) const;

        static float computeFallElevation(float t_passed, bool _boolean, float start_velocy_);

        void Initialize(MovControlType controller, const Vector4& position, uint32 ms_time);

        void Spline_computeElevation(float t_passed, Vector3& position);

        class SplineFace& GetSplineFace() { return (class SplineFace&)*this; }

        MoveSpline& NewSpline();
    };



    class SplineFace : public MovementState
    {
    public:
        // set spline to default state - disabled
        void ResetSplineState();

        void SendPath();

        void UpdateState();
    };

    /// Initializes movement
    class MoveSplineInit
    {
    public:

        explicit MoveSplineInit(MovementState& m) :
            state(m), move(m.move_spline), init2(move), m_new_flags(0) {}

        class SecondInit
        {
            friend class MoveSplineInit;
            MoveSpline&     move;
            explicit SecondInit(MoveSpline& m) : move(m) {}

        public:

            SecondInit& SetKnockBack(float z_acceleration, uint32 start_time);
            SecondInit& SetTrajectory(float z_acceleration, uint32 start_time);
            /// sets final facing animation
            SecondInit& SetFacing(uint64 target_guid);
            SecondInit& SetFacing(float angle);
            SecondInit& SetFacing(Vector3 const& point);
        };

        SecondInit& MovebyPath(const PointsArray& controls);
        SecondInit& MovebyCyclicPath(const PointsArray& controls);
        SecondInit& MoveTo(const Vector3& dest);
        void MoveFall(const Vector3& dest);

        MoveSplineInit& SetFly();
        MoveSplineInit& SetWalk();
        MoveSplineInit& SetSmooth();

        MoveSplineInit& SetVelocy(float velocy);

    private:
        MovementState&  state;
        MoveSpline&     move;
        SecondInit      init2;
        uint32          m_new_flags;

        // lets prevent dynamic allocation
        void* operator new(size_t);
    };


    class MovementAuraFace
    {
    public:

        void WaterWalk(bool on);
        void Hover(bool on);
        void CanFly(bool on);
    };



}
