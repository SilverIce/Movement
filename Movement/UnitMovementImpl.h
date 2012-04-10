
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

    class UnitMovementImpl : public MovingEntity_WOW
    {
    public:

        explicit UnitMovementImpl();

        void Init(Tasks::ITaskExecutor& updater, UnitMovement* publicFace);
        virtual ~UnitMovementImpl();

        void CleanReferences();

        std::string toString() const override;

        Vector3 direction() const;
        Vector3 direction2d() const;
        float directionAngle() const;

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
        Tasks::TaskTarget_DEV commonTasks;

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
        MSTime lastMoveEvent;
    public:
        UnitMoveFlag const moveFlags;
        FloatParameter m_currentSpeedType;

        UnitMovement* PublicFace;

        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused;
    private:

        float m_float_values[Parameter_End];
    };
}
