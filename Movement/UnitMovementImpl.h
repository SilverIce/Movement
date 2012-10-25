
/**
  file:         UnitMovement.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

namespace Movement
{
    class ClientImpl;

    class UnitMovementImpl : public MovingEntity_WOW
    {
    public:

        explicit UnitMovementImpl()
            : m_client(NULL)
        {
            const float BaseValues[Parameter_End] = {
                2.5f,                                                   // SpeedWalk
                7.0f,                                                   // SpeedRun
                4.5f,                                                   // SpeedSwimBack
                4.722222f,                                              // SpeedSwim
                1.25f,                                                  // SpeedRunBack
                7.0f,                                                   // SpeedFlight
                4.5f,                                                   // SpeedFlightBack
                3.141594f,                                              // SpeedTurn
                3.141594f,                                              // SpeedPitch
                2.0f,                                                   // CollisionHeight
                7.0f,                                                   // SpeedCurrent
            };
            memcpy(m_float_values,BaseValues, sizeof m_float_values);
            m_currentSpeedType = Parameter_SpeedRun;
        }

        void Init(Tasks::ITaskExecutor& updater) {
            commonTasks.SetExecutor(updater);
        }

        virtual ~UnitMovementImpl() {
            mov_assert(m_client == NULL);
            mov_assert(!commonTasks.hasExecutor());
        }

        void CleanReferences();

        void toString(QTextStream& st) const override;

        Vector3 direction2d() const {
            if (!moveFlags.hasDirection())
                return Vector3();
            float dir = directionAngle();
            return Vector3(cos(dir), sin(dir), 0);
        }

        Vector3 direction() const {
            if (!moveFlags.hasDirection())
                return Vector3();
            if (!moveFlags.hasFlag(UnitMoveFlag::Mask_Pitching))
                return direction2d();
            float cosPitch = cos(PitchAngle());
            float yaw = directionAngle();
            return Vector3( cosPitch*cos(yaw), cosPitch*sin(yaw), sin(PitchAngle()) );
        }

        float directionAngle() const {
            float dest_angle = YawAngle();
            if (moveFlags.forward) {
                if (moveFlags.strafe_right)
                    dest_angle -= G3D::halfPi()*0.5;
                else if (moveFlags.strafe_left)
                    dest_angle += G3D::halfPi()*0.5;
            }
            else if (moveFlags.backward) {
                dest_angle += G3D::pi();
                if (moveFlags.strafe_right)
                    dest_angle += G3D::halfPi()*0.5;
                else if (moveFlags.strafe_left)
                    dest_angle -= G3D::halfPi()*0.5;
            }
            else if (moveFlags.strafe_right)
                dest_angle -= G3D::halfPi();
            else if (moveFlags.strafe_left)
                dest_angle += G3D::halfPi();
            return dest_angle;
        }

        bool HasMode(MoveMode m) const;

        bool IsWalking() const { return moveFlags.walk_mode;}
        bool IsFlying() const { return moveFlags.hasFlag(UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);}
        bool IsMoving() const { return moveFlags.hasFlag(UnitMoveFlag::Mask_Moving);}
        bool IsTurning() const { return moveFlags.hasFlag(UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);}
        bool IsFalling() const { return moveFlags.falling;}
        bool IsFallingFar() const { return moveFlags.fallingfar;}
        bool SplineEnabled() const { return moveFlags.spline_enabled;}

        float GetCurrentSpeed() const {
            return GetParameter(m_currentSpeedType);
        }

        void SetParameter(FloatParameter p, float value) { m_float_values[p] = value;}
        float GetParameter(FloatParameter p) const { return m_float_values[p];}

        void ApplyMoveFlag(UnitMoveFlag::eUnitMoveFlags f, bool apply) {
            if (apply)
                SetMoveFlag(moveFlags | f);
            else
                SetMoveFlag(moveFlags & ~f);
        }

        void SetMoveFlag(const UnitMoveFlag& newFlags) {
            newFlags.assertValid();
            if ((moveFlags & UnitMoveFlag::Mask_Speed) != (newFlags & UnitMoveFlag::Mask_Speed))
                m_currentSpeedType = SelectSpeedType(newFlags);
            const_cast<UnitMoveFlag&>(moveFlags) = newFlags;
        }

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

        ClientImpl* m_client;
    public:
        MSTime lastMoveEvent;
        UnitMoveFlag const moveFlags;
        FloatParameter m_currentSpeedType;


        /** Data that cames from client. It affects nothing here but might be used in future. */
        _ClientMoveState m_unused;
    private:

        float m_float_values[Parameter_End];
    };
}
