
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
    class MoveSplineUpdatable;
    class ClientImpl;
    struct MoveSplineInitArgs;

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
        friend class MoveSplineUpdatable;

        MoveSplineUpdatablePtr move_spline;

    public:
        bool HasMode(MoveMode m) const;
        void ApplyMoveMode(MoveMode mode, bool apply);

        void Teleport(const Location& loc);

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsFlying() const { return moveFlags.hasFlag(UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}
        bool IsMoving() const { return moveFlags.hasFlag(UnitMoveFlag::Mask_Moving);}
        bool IsTurning() const { return moveFlags.hasFlag(UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFalling() const { return moveFlags.falling;}
        bool IsFallingFar() const { return moveFlags.fallingfar;}
        bool SplineEnabled() const;

        void SetCollisionHeight(float value);
        float GetCollisionHeight() const { return GetParameter(Parameter_CollisionHeight);}

        void SetSpeed(SpeedType type, float s);
        float GetSpeed(SpeedType type) const { return GetParameter((FloatParameter)(0 + type)); }
        float GetCurrentSpeed() const { return GetParameter(Parameter_SpeedCurrent); }

    private:

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
        static SpeedType SelectSpeedType(UnitMoveFlag moveFlags);

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

        void updateRotation();

    private:
        friend class PacketBuilder;

        MoveUpdater* m_updater;
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
