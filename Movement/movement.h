
#pragma once

#include "mov_constants.h"
#include "packet_builder.h"
#include "SplineState.h"
#include "Movelistener.h"

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


        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

        IListener * listener;

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
        bool HasMode(uint32 m) const { return move_mode & (1 << m);}
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

        void Initialize(MovControlType controller, const Vector4& position, uint32 ms_time);

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

    /// Initializer for MoveSpline class
    class MoveSplineInit
    {
    public:

        explicit MoveSplineInit(MovementState& m) :
            state(m), velocity(0.f) { }

        // applyes changes that you have done
        void Apply();

        // Adds movement by parabolic trajectory
        // max_height - the maximum height of parabola, value could be negative and positive
        // start_time - delay between movement starting time and beginning to move by parabolic trajectory
        // you can have only one parabolic motion: previous will be overriden
        MoveSplineInit& SetKnockBack(float max_height, uint32 start_time);
        MoveSplineInit& SetTrajectory(float max_height, uint32 start_time);

        // Adds final facing animation
        // sets unit's facing to specified point/angle/target after all path done
        // you can have only one final facing: previous will be overriden
        MoveSplineInit& SetFacing(uint64 target_guid);
        MoveSplineInit& SetFacing(float angle);
        MoveSplineInit& SetFacing(Vector3 const& point);

        // 
        // controls - array of points, shouldn't be empty
        // is_cyclic_path - if true, makes spline to be initialized as cyclic
        MoveSplineInit& MovebyPath(const PointsArray& controls, bool is_cyclic_path);
        // Initializes spline for simple A to B motion, A is current unit's position, B is destination
        MoveSplineInit& MoveTo(const Vector3& destination);
        // Makes spline to be initialized
        // destination, shold be lower than current unit's position 
        MoveSplineInit& MoveFall(const Vector3& destination);

        // Enables CatmullRom spline interpolation mode(makes path smooth)
        // if not enabled linear spline mode will be chosen
        MoveSplineInit& SetSmooth();
        // Enables CatmullRom spline interpolation mode, enables flying animation
        MoveSplineInit& SetFly();
        // Enables walk mode
        MoveSplineInit& SetWalk();

        // Sets the velocity(in case you want to have custom movement velocity)
        // if no set, speed will be selected based on values from speed table and current movement mode
        // value shouldn't be negative
        MoveSplineInit& SetVelocity(float velocity);

    private:

        MovementState&  state;
        MoveSpline      spline;
        PointsArray     m_path;
        // used for custom speed
        float           velocity;
        // used for trajectory movement
        float           max_vertical_height;       
        //IListener*      listener;

        // lets prevent dynamic allocation: that object should have short lifetime
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
