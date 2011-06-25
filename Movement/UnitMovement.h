
/**
  file:         UnitMovement.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "MovementBase.h"
#include "mov_constants.h"
#include "MoveSplineInit.h"
#include "ClientMoveStatus.h"

namespace Movement
{
    class MoveSpline;
    class Client;

    // Manages by sequential set of client movement states
    class MoveStateSet
    {
        // TODO: more memory efficient storage
        std::list<ClientMoveState> m_state_queue;
        const ClientMoveState& LastQueuedState() const { return m_state_queue.front();}     // may crash in case no states queued
        const ClientMoveState& CurrentState() const { return m_state_queue.back();}     // may crash in case no states queued
    public:
        explicit MoveStateSet() {}

        void QueueState(const ClientMoveState& state) { m_state_queue.push_front(state);}
        bool Next(ClientMoveState& state, MSTime time_now);
        void Clear() { m_state_queue.clear();}
        size_t Size() const { return m_state_queue.size();}
    };

    // class for unit's movement
    class UnitMovement : public Transportable, public IUpdatable
    {
    public:

        explicit UnitMovement(WorldObjectType owner);
        virtual ~UnitMovement();

        virtual void CleanReferences();
        virtual void UpdateState();

        std::string ToString() const;

        void Initialize(const Location& position, MoveUpdater& updater);
        Location AssumePosition(float time) const;

        /* Needed for monster movement only*/
        void BindOrientationTo(MovementBase& target);
        void UnbindOrientation();
        bool IsOrientationBinded() const { return m_target_link.linked(); }
        const MovementBase* GetTarget() const { return m_target_link.Value.target;}

        const Location& GetPosition() const { return *managed_position;}
        const Vector3& GetPosition3() const { return *managed_position;}
        Vector3 direction() const;

    public:
        virtual void BoardOn(const TransportData& m, const Location& local_position);
        void BoardOn(const TransportData& transport, const Location& local_position, int8 seatId);
        virtual void Unboard();

        /*void Board(Transportable& ps, const Location& local_pos)
        {
            ps.BoardOn(TransportData(this,&m_transport),local_pos);
        }*/

        void Board(UnitMovement& ps, const Location& local_pos, int8 seat)
        {
            ps.BoardOn(TransportData(this,&m_transport),local_pos, seat);
        }

        void UnboardAll() { m_transport.Iterate(Transport::Unboarder());}

    public:
        // Used by server side controlled movement
        void LaunchMoveSpline(MoveSplineInitArgs& args);
        uint32 MoveSplineId() const;
        const Vector3& MoveSplineDest() const;
        int32 MoveSplineTimeElapsed() const;

    public:
        /** Seems it should be removed(or used for monster movement only), since it hard or impossible to get movement mode from incoming movement packets*/
        /// Move Modes
        bool HasMode(MoveMode m) const { return move_mode & (1 << m);}
        void ApplyMoveMode(MoveMode mode, bool apply);

        /// Apply/remove modes
        void ApplyRootMode(bool apply);
        void ApplySwimMode(bool apply) { ApplyMoveMode(MoveModeSwim, apply); }
        void ApplyWalkMode(bool apply) { ApplyMoveMode(MoveModeWalk, apply); }
        void ApplyWaterWalkMode(bool apply) { ApplyMoveMode(MoveModeWaterwalk, apply); }
        void ApplySlowFallMode(bool apply) { ApplyMoveMode(MoveModeSlowfall, apply); }
        void ApplyFlyMode(bool apply) { ApplyMoveMode(MoveModeFly, apply); }
        void ApplyHoverMode(bool apply) { ApplyMoveMode(MoveModeHover, apply); }

        void Teleport(const Location& loc);

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsMoving() const { return moveFlags & UnitMoveFlag::Mask_Moving;}
        bool IsTurning() const { return moveFlags & (UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFlying() const { return moveFlags & (UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}

        void SetCollisionHeight(float value);
        float GetCollisionHeight() const { return GetParameter(Parameter_CollisionHeight);}

        void SetSpeed(SpeedType type, float s);
        float GetSpeed(SpeedType type) const { return GetParameter((FloatParameter)(0 + type)); }
        float GetCurrentSpeed() const { return GetParameter(Parameter_SpeedCurrent); }
        SpeedType getCurrentSpeedType() const { return speed_type; }

        uint32 dbg_flags;

        #pragma region Impl
    private:
        enum{
        /** Affects spline movement precision & performance,
            makes spline movement to be updated once per N milliseconds. */
            MoveSpline_UpdateDelay = 200,

        /** Upper limit for diff time values, milliseconds. Useful when movement wasn't get updated for a long time. */
            Maximum_update_difftime = 10000,
        };

        struct MoveSplineUpdater; 

        void setLastUpdate(MSTime time) { last_update_time = time;}
        MSTime getLastUpdate() const { return last_update_time;}

        bool HasUpdater() const { return updatable.HasUpdater();}
        MoveUpdater& GetUpdater() const { return updatable.GetUpdater();}

        void ApplyState(const ClientMoveState& state);
        void ApplyMoveMode_NoBroadcast(MoveMode mode, bool apply);
    public:
        enum FloatParameter
        {
            Parameter_SpeedWalk,
            Parameter_SpeedRun,
            Parameter_SpeedSwimBack,
            Parameter_SpeedSwim,
            Parameter_SpeedRunBack,
            Parameter_SpeedFlight,
            Parameter_SpeedFlightBack,
            Parameter_SpeedTurn,
            Parameter_SpeedPitch,
            Parameter_CollisionHeight,
            Parameter_SpeedCurrent,
            Parameter_End,
        };

        void SetParameter(FloatParameter p, float value) { m_float_values[p] = value;}
        float GetParameter(FloatParameter p) const { return m_float_values[p];}

        void _QueueState(const ClientMoveState& state) { m_moveEvents.QueueState(state);}       // only for call from Client code
        ClientMoveState ClientState() const;

        void SetPosition(const Location& v);
        void SetPosition(const Vector3& v) { SetPosition(Location(v,managed_position->orientation));}

        Client* client() const { return m_client;}
        void client(Client* c) { m_client = c;}
        bool IsClientControlled() const { return GetControl() == MovControlClient;}
    private:
        void ReCalculateCurrentSpeed();
        static SpeedType SelectSpeedType(UnitMoveFlag moveFlags);

        MovControlType GetControl() const
        {
            const MovControlType tt[] = {MovControlClient,MovControlServer};
            return tt[(SplineEnabled() || !m_client)];
        }

        bool IsServerControlled() const { return GetControl() == MovControlServer;}
        bool SplineEnabled() const { return moveFlags.spline_enabled; }
        void DisableSpline() { moveFlags &= ~(UnitMoveFlag::Mask_Directions | UnitMoveFlag::Spline_Enabled);}
        void PrepareMoveSplineArgs(MoveSplineInitArgs&, UnitMoveFlag&, SpeedType&) const;

        void updateRotation();
        void reset_managed_position() { managed_position = (Location*)&GetGlobalPosition();}

    private:
        friend class PacketBuilder;
        //friend class Client;
        friend class IModifier;
        friend struct CMovement;

        UpdatableMovement updatable;
        Location * managed_position;
        MoveSpline& move_spline;
        Client* m_client;
        MSTime last_update_time;
        MoveStateSet m_moveEvents;
        Transport m_transport;

        UnitMoveFlag moveFlags;
        uint32 move_mode;
        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused; 

        LinkedListElement<TargetLink> m_target_link;
        SpeedType speed_type;
        float m_float_values[Parameter_End];
        #pragma endregion
    };


    class IModifier
    {
        UnitMovement& mov;
    public:

        IModifier(UnitMovement& m) : mov(m) {}

        void SetCollisionHeight(float value);
        void SetSpeed(SpeedType type, float s);
        void ApplyMoveMode(MoveMode mode, bool apply);
    };


    /*
    Who interacts with movement?
        - movestates from client -> session-> queue state
    core:
        chain: server pck-> client's answer

        sometimes need send message to client-controller (state has changed, knockback opcode maybe..)

        Auras wants to:
            - apply or remove hover, waterwalk, fly, root, slow fall etc modes
        Movegens want to:
            - launch spline movement by some path, with some addit. parameters (falling, parabolic etc)
        Session(client) wants to:
            handle messages from client, apply movestates from client, got an answers at some requests
            sometimes we want to: change state and send some packet to client

        There are three message types. These messages are needed to:
            - broadcasted to all clients
            - sended only to our client
            - receivee and handlee somehow

        Two kinds of message chains:
            smsg(server is initiator) -> cmsg(client answers)
            cmsg(client is initiator) -> smsg(client answers)

            struct ClientRequests
            {
            void handleAPacket();
            void HandleBPacket();
            void HandleCPacket();
            }

            struct MaNGOSRequests
            {
            void TeleportTo(); // 2 modes
            void KnockBack(); // 2

            void ApplyMode();
            }

    */

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

        static OnEventArgs OnPoint(uint32 splineId, int32 pointId)
        {
            OnEventArgs args = {PointDone, splineId, pointId};
            return args;
        }

        bool isArrived() const { return type == Arrived;}
        bool isPointDone() const { return type == PointDone;}

        EventType type;
        uint32 splineId;
        int32 data;
    };

/*
    class IUnitMovement
    {
        // Used by server side controlled movement
        void EnableSpline() { moveFlags.spline_enabled = true; }
        void DisableSpline() { moveFlags.spline_enabled = false; }
        bool SplineEnabled() const { return moveFlags.spline_enabled; }
        void CleanDirection()  { moveFlags &= ~UnitMoveFlag::Mask_Directions; }
        void SetForwardDirection() { moveFlags.forward = true; }

    public:

        virtual void UpdateState();

        virtual void SetSpeed(SpeedType type, float s);
        virtual void ApplyMoveMode(MoveMode mode, bool apply);

        virtual void EnterTransport(Transport& t, Location offset, int8 seatId = -1);

        void BindOrientationTo(MovementBase& target);
        void UnbindOrientation();
        bool IsOrientationBinded() const { return m_target_link; }
        MovementBase* GetTarget() { return m_target_link.Value.target;}
        const MovementBase* GetTarget() const { return m_target_link.Value.target;}
    };
*/
}
