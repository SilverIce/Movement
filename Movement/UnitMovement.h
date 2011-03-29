
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
#include "MoveSplineInit.h"
//#include "UnitMoveFlags.h"
#include "ClientMoveStatus.h"

namespace Movement
{
    class MoveSplineSegmented;

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

    // class for unit's movement
    class UnitMovement : public Transportable
    {
    public:

        virtual void CleanReferences()
        {
            UnbindOrientation();
            m_transport.CleanReferences();
            Transportable::CleanReferences();
        }

        #pragma region Orientation
        /* Needed for monster movement only*/
    public:
        void BindOrientationTo(MovementBase& target);
        void UnbindOrientation();
        bool IsOrientationBinded() const { return m_target_link.linked(); }
        //MovementBase* GetTarget() { return m_target_link.Value.target;}
        const MovementBase* GetTarget() const { return m_target_link.Value.target;}
    private:
        void updateRotation();
        LinkedListElement<TargetLink> m_target_link;
        #pragma endregion

        #pragma region Transport
    public:
        virtual void Board(Transport& m) {_board(m);}
        virtual void UnBoard() {_unboard();}

    private:
        // Does all units are transporters?
        // if not, need add subclass or allocate it dynamically
        Transport m_transport;
        #pragma endregion

        friend class PacketBuilder;
        friend class MoveSplineInit;

    public:
        explicit UnitMovement(WorldObject& owner);
        virtual ~UnitMovement();

        void SetControl(MovControlType c) { control_mode = c; }
        MovControlType GetControl() const { return control_mode; }

        void EnableSpline() { moveFlags.spline_enabled = true; }
        void DisableSpline() { moveFlags.spline_enabled = false; }
        bool SplineEnabled() const { return moveFlags.spline_enabled; }

        bool HasDest() const { return moveFlags.hasDirection(); }
        void ResetDirection()  { moveFlags &= ~UnitMoveFlag::Mask_Directions; }
        void SetForwardDirection() { moveFlags.forward = true; }

        uint32 GetMovementFlags() const { return moveFlags.raw; }

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

    private:

        #pragma region fields


        #pragma region Speed
    public:
        void SetSpeed(SpeedType type, float s) { speed[type] = s; }
        float GetSpeed(SpeedType type) const { return speed[type]; }
        float GetCurrentSpeed() const { return speed_obj.current; }
        SpeedType getCurrentSpeedType() const { return speed_type; }
        void ReCalculateCurrentSpeed();
        SpeedType SelectSpeedType(bool use_walk_forced) const;
    private:
        SpeedType speed_type;
        MovControlType  control_mode;

        uint32          last_ms_time;
        uint32          move_mode;

        UnitMoveFlag    moveFlags;
        UnitMoveFlag2   moveFlags2;

        // swimming and flying
        float           pitch;
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
        #pragma endregion



    public:
        uint8           dbg_flags;

        MoveSplineSegmented&  move_spline;

        virtual void UpdateState();



        void Initialize(MovControlType controller, const Location& position);
        void ApplyState(const ClientMoveState& );

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
        UnitMovement& impl;
    public:

        Scketches(UnitMovement* m) : impl(*m) {}

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
