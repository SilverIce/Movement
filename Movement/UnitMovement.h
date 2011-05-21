
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
    class MoveSpline;

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

        explicit UnitMovement(WorldObject& owner);
        virtual ~UnitMovement();

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
        virtual void BoardOn(Transport& transport, const Location& local_position, int8 seatId);
        virtual void Unboard();

        void Board(Transportable& t, const Location& local_position, int8 seatId) { t.BoardOn(m_transport, local_position, seatId);}
    private:
        // Does all units are transporters?
        // if not, need add subclass or allocate it dynamically
        Transport m_transport;
        #pragma endregion

        friend class PacketBuilder;

    public:

        void SetControl(MovControlType c) { control_mode = c; }
        MovControlType GetControl() const { return control_mode; }

        void EnableSpline() { moveFlags.spline_enabled = true; }
        void DisableSpline() { moveFlags.spline_enabled = false; }
        bool SplineEnabled() const { return moveFlags.spline_enabled; }
        void LaunchMoveSpline(MoveSplineInitArgs& args);

    private:
        void PrepareMoveSplineArgs(MoveSplineInitArgs&,UnitMoveFlag&, SpeedType&) const;
        void CleanDirectionFlags()  { moveFlags &= ~UnitMoveFlag::Mask_Directions; }

        #pragma region Move modes
    public:
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

        bool IsMoving() const { return moveFlags & UnitMoveFlag::Mask_Moving;}
        bool IsTurning() const { return moveFlags & (UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}

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
        static SpeedType SelectSpeedType(UnitMoveFlag moveFlags);
    private:
        SpeedType speed_type;
        union {
            SpeedInfo   speed_obj;
            float       speed[SpeedMaxCount];
        };
        #pragma endregion

    private:
        MovControlType control_mode;

        UnitMoveFlag moveFlags;
        uint32 last_update_time;
        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused;

        enum{
        /** Affects spline movement precision & performance,
            makes spline movement to be updated once per N milliseconds. */
            MoveSpline_UpdateDelay = 400,
        };

        struct MoveSplineUpdater; 

        void setLastUpdate(uint32 time) { last_update_time = time;}
        uint32 getLastUpdate() const { return last_update_time;}

    public:
        uint8           dbg_flags;
        std::string ToString() const;

        MoveSpline&  move_spline;

        virtual void UpdateState();



        void Initialize(MovControlType controller, const Location& position, MoveUpdater& updater);
        void ApplyState(const ClientMoveState& );

    };

    struct MsgBroadcast : public MsgDeliverer
    {
        explicit MsgBroadcast(WorldObject& owner) : m_owner(owner) {}
        explicit MsgBroadcast(MovementBase* m) : m_owner(m->Owner) {}
        explicit MsgBroadcast(MovementBase& m) : m_owner(m.Owner) {}
        virtual void operator()(WorldPacket& data);
        WorldObject& m_owner;
    };

    struct MsgBroadcastExcept : public MsgDeliverer
    {
        explicit MsgBroadcastExcept(WorldObject& owner, WorldObject& except) : m_owner(owner), m_except(except) {}
        explicit MsgBroadcastExcept(MovementBase* m, WorldObject& except) : m_owner(m->Owner), m_except(except) {}
        virtual void operator()(WorldPacket& data);
        WorldObject& m_owner;
        WorldObject& m_except;
    };

    struct OnEventArgs
    {
        enum EventType{
            PointDone,
            Arrived,
        };

        static OnEventArgs OnArrived(uint32 splineId)
        {
            OnEventArgs args = {Arrived, splineId, 0};
            return args;
        }

        static OnEventArgs OnPoint(uint32 splineId, int pointId)
        {
            OnEventArgs args = {PointDone, splineId, pointId};
            return args;
        }

        bool isArrived() const { return type == Arrived;}
        bool isPointDone() const { return type == PointDone;}

        EventType type;
        uint32 splineId;
        int data;
    };

}
