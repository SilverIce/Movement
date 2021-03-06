
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

        explicit UnitMovementImpl(WorldObjectType owner, uint64 ownerGuid, MoveUpdater& updater);
        virtual ~UnitMovementImpl();

        virtual void CleanReferences();

        std::string ToString() const;

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
        friend class MoveSplineUpdatable;

        MoveSplineUpdatablePtr move_spline;

    public:
        bool HasMode(MoveMode m) const;

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsFlying() const { return moveFlags.hasFlag(UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}
        bool IsMoving() const { return moveFlags.hasFlag(UnitMoveFlag::Mask_Moving);}
        bool IsTurning() const { return moveFlags.hasFlag(UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFalling() const { return moveFlags.falling;}
        bool IsFallingFar() const { return moveFlags.fallingfar;}
        bool SplineEnabled() const;

        float GetCurrentSpeed() const { return GetParameter(SelectSpeedType(moveFlags));}

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
        static FloatParameter SelectSpeedType(UnitMoveFlag moveFlags);

        ClientMoveState ClientState() const;
        void ApplyState(const ClientMoveState& state);

        bool HasUpdater() const { return m_updater != NULL;}
        MoveUpdater& Updater() const { return *m_updater;}
        TaskTarget_DEV commonTasks;

        ClientImpl* client() const { return m_client;}
        void client(ClientImpl* c) {
            assert_state(!m_client || (m_client && !c));
            m_client = c;
        }
        bool IsClientControlled() const { return m_client && !SplineEnabled();}
        bool IsServerControlled() const { return !m_client;}

    private:
        friend class PacketBuilder;

        MoveUpdater* m_updater;
        ClientImpl* m_client;
        MSTime lastMoveEvent;
    public:
        UnitMoveFlag const moveFlags;
    private:
        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused; 

        float m_float_values[Parameter_End];
        TaskTarget m_updateRotationTask;
        LinkedListElement<TargetLink> m_target_link;
        LinkedList<TargetLink> m_targeter_references;
    };
}
