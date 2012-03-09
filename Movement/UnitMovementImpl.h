
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

    // class for unit's movement
    class UnitMovementImpl : public ComponentT<UnitMovementImpl>
    {
    public:

        explicit UnitMovementImpl();

        void Init(Component& tree, Tasks::ITaskExecutor& updater, UnitMovement* publicFace);
        virtual ~UnitMovementImpl();

        void CleanReferences();

        std::string ToString() const;

        Vector3 direction() const;

        float directionAngle() const;

    public:

        // TODO: too much aliases here..

        Location GetGlobalPosition() const {
            return Location(m_entity->GlobalPosition(),m_entity->YawAngle());
        }

        const Vector3& GetGlobalPosition3() const {
            return m_entity->GlobalPosition();
        }

        const Vector3& GetRelativePosition() const { return m_entity->RelativePosition();}

        void SetRelativePosition(const Vector3& position)
        {
            m_entity->RelativePosition(position);
            lastPositionChangeTime = Imports.getMSTime();
            Imports.OnPositionChanged(Owner, GetGlobalPosition());
        }

        void SetRelativePosition(const Location& position)
        {
            m_entity->RelativePosition(position);
            m_entity->YawAngle(position.orientation);
            lastPositionChangeTime = Imports.getMSTime();
            Imports.OnPositionChanged(Owner, GetGlobalPosition());
        }

        void SetOrientation(float orientation)
        {
            m_entity->YawAngle(orientation);
            Imports.OnPositionChanged(Owner, GetGlobalPosition());
        }

        float GetOrientation() const { return m_entity->YawAngle();}

        void PitchAngle(float pitchAngle) { m_entity->PitchAngle(pitchAngle);}
        float PitchAngle() const { return m_entity->PitchAngle();}

    public:
        bool HasMode(MoveMode m) const;

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsFlying() const { return moveFlags.hasFlag(UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}
        bool IsMoving() const { return moveFlags.hasFlag(UnitMoveFlag::Mask_Moving);}
        bool IsTurning() const { return moveFlags.hasFlag(UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFalling() const { return moveFlags.falling;}
        bool IsFallingFar() const { return moveFlags.fallingfar;}
        bool SplineEnabled() const;

        float GetCurrentSpeed() const {
            return GetParameter(m_currentSpeedType);
        }

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

        Tasks::ITaskExecutor& Updater() const { return *commonTasks.getExecutor();}
        TaskTarget_DEV commonTasks;

        ClientImpl* client() const { return m_client;}
        void client(ClientImpl* c) {
            assert_state(!m_client || !c);
            m_client = c;
        }
        bool IsClientControlled() const { return m_client && !SplineEnabled();}

    private:
        friend class PacketBuilder;

        void assertCleaned() const;

        ClientImpl* m_client;
        MovingEntity_Revolvable2 * m_entity;
        MSTime lastMoveEvent;
    public:
        UnitMoveFlag const moveFlags;
        FloatParameter m_currentSpeedType;
        MSTime lastPositionChangeTime;

        UnitMovement* PublicFace;
        WorldObject* Owner;
        ObjectGuid Guid;

        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused;
    private:

        float m_float_values[Parameter_End];
    };
}
