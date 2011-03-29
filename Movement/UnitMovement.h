
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
    class UnitMovement : public Transportable, public IUpdatable
    {
    public:

        virtual void CleanReferences()
        {
            UnbindOrientation();
            m_transport.CleanReferences();
            Transportable::CleanReferences();
            updatable.CleanReferences();
        }

        #pragma region Updates
    public:
        bool HasUpdater() const { return updatable.HasUpdater();}
        void SetUpdater(MoveUpdater& upd) { updatable.SetUpdater(upd);}
        MoveUpdater& GetUpdater() const { return updatable.GetUpdater();}
        void ScheduleUpdate() { updatable.ScheduleUpdate();}
    private:
        UpdatableMovement updatable;
        #pragma endregion

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

        #pragma region Move modes
        /// Move Modes
        bool HasMode(MoveMode m) const { return move_mode & (1 << m);}
        void ApplyMoveMode(MoveMode mode, bool apply);
        /// end of Get-Set methods

        /// Apply/remove modes
        void ApplyRootMode(bool apply) { ApplyMoveMode(MoveModeRoot, apply); }
        void ApplySwimMode(bool apply) { ApplyMoveMode(MoveModeSwim, apply); }
        void ApplyWalkMode(bool apply) { ApplyMoveMode(MoveModeWalk, apply); }
        void ApplyWaterWalkMode(bool apply) { ApplyMoveMode(MoveModeWaterwalk, apply); }
        void ApplySlowFallMode(bool apply) { ApplyMoveMode(MoveModeSlowfall, apply); }
        void ApplyFlyMode(bool apply) { ApplyMoveMode(MoveModeFly, apply); }
        void ApplyHoverMode(bool apply) { ApplyMoveMode(MoveModeHover, apply); }

    private:
        uint32 move_mode;
        #pragma endregion

        #pragma region Speed
    public:
        void SetSpeed(SpeedType type, float s);
        float GetSpeed(SpeedType type) const { return speed[type]; }
        float GetCurrentSpeed() const { return speed_obj.current; }
        SpeedType getCurrentSpeedType() const { return speed_type; }
        void ReCalculateCurrentSpeed();
        SpeedType SelectSpeedType(bool use_walk_forced) const;
    private:
        SpeedType speed_type;
        MovControlType  control_mode;

        uint32          last_ms_time;

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

        TransportInfo   m_transportInfo;

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
        explicit MsgBroadcast(MovementBase* m) : m_owner(m->Owner) {}
        explicit MsgBroadcast(MovementBase& m) : m_owner(m.Owner) {}
        virtual void operator()(WorldPacket& data) { m_owner.SendMessageToSet(&data, true);}
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
