
/**
  file:         UnitMovement.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    class MoveSpline;
    class ClientImpl;
    struct MoveSplineInitArgs;

    using Tasks::TaskTarget;
    using Tasks::TaskExecutor_Args;
    using Tasks::CallBack;
    using Tasks::CallBackPublic;
    using Tasks::StaticExecutor;
    using Tasks::Executor;
    using Tasks::TaskTarget_DEV;

    struct MsgBroadcast : public MsgDeliverer
    {
        explicit MsgBroadcast(WorldObjectType owner) : m_owner(owner) {}
        explicit MsgBroadcast(MovementBase* m) : m_owner(m->Owner) {}
        explicit MsgBroadcast(MovementBase& m) : m_owner(m.Owner) {}
        virtual void operator()(WorldPacket& data) { MaNGOS_API::BroadcastMessage(&m_owner, data);}
        WorldObjectType m_owner;
    };

    struct TargetLink
    {
        TargetLink() : target(0), targeter(0) {}
        TargetLink(UnitMovementImpl* target_, UnitMovementImpl* targeter_)
            : target(target_), targeter(targeter_) {}

        UnitMovementImpl* target;
        UnitMovementImpl* targeter;
    };

    // class for unit's movement
    class UnitMovementImpl : public MovementBase
    {
    public:

        explicit UnitMovementImpl(WorldObjectType owner);
        virtual ~UnitMovementImpl();

        virtual void CleanReferences();
        void UpdateState(MSTime timeNow);

        std::string ToString() const;

        void Initialize(const Location& position, MoveUpdater& updater);

        /* Needed for monster movement only*/
        void BindOrientationTo(UnitMovementImpl& target);
        void UnbindOrientation();
        bool IsOrientationBinded() const { return m_target_link.linked(); }
        const UnitMovementImpl* GetTarget() const { return m_target_link.Value.target;}

        Vector3 direction() const;

    public:
        const Location& GetLocalPosition() const { return world_position;}
        const Location& GetPosition() const { return world_position;}
        const Vector3& GetPosition3() const { return world_position;}
        void SetPosition(const Location& v) { SetGlobalPosition(v); }

        bool IsBoarded() const { return false;}
        MovementBase* GetTransport() const { mov_assert(false); return NULL; }

    public:
        // Used by server side controlled movement
        void LaunchMoveSpline(MoveSplineInitArgs& args);
        uint32 MoveSplineId() const;
        const Vector3& MoveSplineDest() const;
        int32 MoveSplineTimeElapsed() const;
        void DisableSpline() { ApplyMoveFlag(UnitMoveFlag::Mask_Moving|UnitMoveFlag::Spline_Enabled,false);}

        void SetListener(IListener * l) { m_listener = l;}
        void ResetLisener() { m_listener = NULL; }

    public:
        bool HasMode(MoveMode m) const;
        void ApplyMoveMode(MoveMode mode, bool apply);

        void Teleport(const Location& loc);

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsFlying() const { return moveFlags.hasFlag(UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}
        bool IsMoving() const { return moveFlags.hasFlag(UnitMoveFlag::Mask_Moving);}
        bool SplineEnabled() const { return moveFlags.spline_enabled; }
        bool IsTurning() const { return moveFlags.hasFlag(UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFalling() const { return moveFlags.falling;}
        bool IsFallingFar() const { return moveFlags.fallingfar;}

        void SetCollisionHeight(float value);
        float GetCollisionHeight() const { return GetParameter(Parameter_CollisionHeight);}

        void SetSpeed(SpeedType type, float s);
        float GetSpeed(SpeedType type) const { return GetParameter((FloatParameter)(0 + type)); }
        float GetCurrentSpeed() const { return GetParameter(Parameter_SpeedCurrent); }

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

    public:

        void SetParameter(FloatParameter p, float value) { m_float_values[p] = value;}
        float GetParameter(FloatParameter p) const { return m_float_values[p];}

        void ApplyMoveFlag(UnitMoveFlag::eUnitMoveFlags f, bool apply)
        {
            if (apply)
                SetMoveFlag(moveFlags | f);
            else
                SetMoveFlag(moveFlags & ~f);
        }

        void SetMoveFlag(const UnitMoveFlag& newFlags);

        ClientMoveState ClientState() const;
        void ApplyState(const ClientMoveState& state);

        bool HasUpdater() const { return m_updater != NULL;}
        MoveUpdater& Updater() const { return *m_updater;}
        TaskTarget_DEV commonTasks;

        ClientImpl* client() const { return m_client;}
        void client(ClientImpl* c) { m_client = c;}
        bool IsClientControlled() const { return m_client && !SplineEnabled();}
        bool IsServerControlled() const { return !m_client;}
    private:
        static SpeedType SelectSpeedType(UnitMoveFlag moveFlags);

        void PrepareMoveSplineArgs(MoveSplineInitArgs&, UnitMoveFlag&) const;
        void updateRotation();

    private:
        friend class PacketBuilder;

        MoveUpdater* m_updater;
        MoveSpline* move_spline;
        IListener* m_listener;
        ClientImpl* m_client;
        MSTime last_update_time;

        UnitMoveFlag const moveFlags;
        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused; 

        float m_float_values[Parameter_End];
        LinkedListElement<TargetLink> m_target_link;
        LinkedList<TargetLink> m_targeter_references;
    };
}
