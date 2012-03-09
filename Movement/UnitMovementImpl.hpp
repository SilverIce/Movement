#pragma once

namespace Movement
{
    FloatParameter UnitMovementImpl::SelectSpeedType(UnitMoveFlag moveFlags)
    {
        if (moveFlags.spline_enabled)
            return Parameter_SpeedCustom;

        bool use_walk_forced = false;
        if ( moveFlags.flying )
        {
            if ( moveFlags.backward /*&& speed_obj.flight >= speed_obj.flight_back*/ )
                return Parameter_SpeedFlightBack;
            else
                return Parameter_SpeedFlight;
        }
        else if ( moveFlags.swimming )
        {
            if ( moveFlags.backward /*&& speed_obj.swim >= speed_obj.swim_back*/ )
                return Parameter_SpeedSwimBack;
            else
                return Parameter_SpeedSwim;
        }
        else
        {
            if ( moveFlags.walk_mode || use_walk_forced )
            {
                //if ( speed_obj.run > speed_obj.walk )
                    return Parameter_SpeedWalk;
            }
            else
            {
                if ( moveFlags.backward /*&& speed_obj.run >= speed_obj.run_back*/ )
                    return Parameter_SpeedRunBack;
            }
            return Parameter_SpeedRun;
        }
    }

    UnitMovementImpl::UnitMovementImpl() :
        Owner(NULL),
        m_updater(NULL),
        m_entity(NULL),
        PublicFace(NULL),
        m_client(NULL)
    {
        const float BaseValues[Parameter_End] =
        {
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

    void UnitMovementImpl::Init(Component& tree, MoveUpdater& updater, UnitMovement* publicFace)
    {
        tree.ComponentAttach(this);

        WowObject * wobj = getAspect<WowObject>();
        Owner = wobj->object;
        Guid = wobj->guid;

        m_updater = &updater;
        PublicFace = publicFace;
        m_entity = getAspect<MovingEntity_Revolvable2>();

        commonTasks.SetExecutor(updater);
    }

    void UnitMovementImpl::assertCleaned() const
    {
        mov_assert(m_client == NULL);
        mov_assert(m_updater == NULL);
        mov_assert(!commonTasks.hasExecutor());
    }

    UnitMovementImpl::~UnitMovementImpl()
    {
        assertCleaned();
    }

    void UnitMovementImpl::CleanReferences()
    {
        if (m_client) {
            m_client->Dereference(this);
            m_client = NULL;
        }

        commonTasks.Unregister();
        m_updater = NULL;
        m_entity = nullptr;
        PublicFace = nullptr;
        Owner = nullptr;
    }

    Vector3 UnitMovementImpl::direction() const
    {
        if (!moveFlags.hasDirection())
            return Vector3();

        float dir = directionAngle();
        return Vector3(cos(dir), sin(dir), 0);
    }

    float UnitMovementImpl::directionAngle() const
    {
        float dest_angle = GetOrientation();
        if (moveFlags.forward)
        {
            if (moveFlags.strafe_right)
                dest_angle -= G3D::halfPi()*0.5;
            else if (moveFlags.strafe_left)
                dest_angle += G3D::halfPi()*0.5;
        }
        else if (moveFlags.backward)
        {
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


    std::string UnitMovementImpl::ToString() const
    {
        std::stringstream st;
        st << PacketBuilder::CreateMoveState(*this).ToString();
        if (m_client)
            st << m_client->ToString();
        if (SplineEnabled())
            st << getAspect<MoveSplineUpdatable>()->ToString();
        return st.str();
    }

    bool UnitMovementImpl::SplineEnabled() const {
        return moveFlags.spline_enabled;
    }

    void validateFLags(const UnitMoveFlag& newFlags)
    {
        // Some movement flag combinations freeze the client
        // asserts that flags aren't enabled in same time

        assert_state( (newFlags.forward & newFlags.backward) == 0 );
        assert_state( (newFlags.strafe_left & newFlags.strafe_right) == 0);
        assert_state( (newFlags.ascending & newFlags.descending) == 0);
        assert_state( (newFlags.pitch_up & newFlags.pitch_down) == 0);

        //assert_state( (newFlags.spline_enabled & newFlags.root) == 0); //confirmed
    }

    void UnitMovementImpl::SetMoveFlag(const UnitMoveFlag& newFlags)
    {
        validateFLags(newFlags);

        if ((moveFlags & UnitMoveFlag::Mask_Speed) != (newFlags & UnitMoveFlag::Mask_Speed))
            m_currentSpeedType = UnitMovementImpl::SelectSpeedType(newFlags);
        const_cast<UnitMoveFlag&>(moveFlags) = newFlags;
    }
}
