
#pragma once

#include "typedefs.h"
#include "mov_constants.h"
#include "packet_builder.h"
#include "SplineState.h"
#include "G3D\Vector3.h"
#include "G3D\Vector4.h"

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
        void Stop()  { direction_flags &= ~DIRECTIONS_MASK; }

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

        uint32          last_ms_time_fake;

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

        MoveSpline     splineInfo;

        #pragma endregion


        /// Some client's formulas:

        void ReCalculateCurrentSpeed();
        float CalculateCurrentSpeed(bool use_walk_forced) const;

        static float computeFallElevation(float t_passed, bool _boolean, float start_velocy_);

        void Initialize(MovControlType controller, Vector4& position);

        void Spline_computeElevation(float t_passed, Vector3& position);

        void MovebyPath(const Vector3*, int count, float speed, bool cyclic);
        void MovebyPath(const Vector3*, int count, bool cyclic);

        void UpdatePosition(uint32 curr_ms_time, Vector3 & c);
        void UpdatePositionWithTickDiff(uint32 diff, Vector3 & c);

    };



}
