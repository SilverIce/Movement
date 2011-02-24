
/**
  file:         UnitMovement.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "MovementBase.h"
#include "mov_constants.h"
#include "packet_builder.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"

namespace Movement
{
    class MoveSplineSegmented;

    class MovementState : public UnitBase
    {
        friend class PacketBuilder;

    public:
        MovementState(WorldObject * owner);

        ~MovementState()
        {
        }

        void SetControl(MovControlType c) { control_mode = c; }
        MovControlType GetControl() const { return control_mode; }

        #pragma region field accessors

        /// Get-Set methtods

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
            float walk;
            float run;
            float run_back;
            float swim;
            float swim_back;
            float flight;
            float flight_back;
            float turn;
            float pitch;
            float current;
        };

        MovControlType  control_mode;

        uint32          last_ms_time;
        uint32          move_mode;

        union {
            uint32      moveFlags;
            uint8       direction_flags;
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


        SpeedType       speed_type;

        void updateRotation();

        #pragma endregion

    public:

        MoveSplineUsed  move_spline;

        virtual void UpdateState();

        void BindOrientationTo(MovementBase& target);
        void UnbindOrientation();

        /// Speed
        void SetSpeed(SpeedType type, float s) { speed[type] = s; }
        float GetSpeed(SpeedType type) const { return speed[type]; }
        float GetCurrentSpeed() const { return speed_obj.current; }
        SpeedType getCurrentSpeedType() const { return speed_type; }
        void ReCalculateCurrentSpeed();
        SpeedType SelectSpeedType(bool use_walk_forced) const;

        void Initialize(MovControlType controller, const Vector4& position, uint32 ms_time);

        friend class Scketches;
    };

    struct MsgBroadcast : public MsgDeliverMethtod
    {
        explicit MsgBroadcast(WorldObject& owner) : m_owner(owner) {}
        explicit MsgBroadcast(MovementBase* m) : m_owner(m->GetOwner()) {}
        explicit MsgBroadcast(MovementBase& m) : m_owner(m.GetOwner()) {}
        virtual void operator()(WorldPacket& data);
        WorldObject& m_owner;
    };

    class Scketches
    {
        MovementState& impl;
    public:

        Scketches(MovementState* m) : impl(*m) {}

        void ForceStop();

        void SendPath()
        {
            PacketBuilder::PathUpdate(impl, MsgBroadcast(&impl));
        }

    };

    class MovementAuraFace
    {
    public:

        void WaterWalk(bool on);
        void Hover(bool on);
        void CanFly(bool on);
    };
}
