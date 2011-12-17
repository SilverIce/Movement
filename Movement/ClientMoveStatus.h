#pragma once

namespace Movement
{
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
        Parameter_SpeedMoveSpline,
        Parameter_End,
    };

    /** Contains unused fields */
    struct _ClientMoveState
    {
        _ClientMoveState() : pitchAngle(0), fallTime(0),
            jump_verticalVelocity(0), jump_directionY(0), jump_directionX(0), jump_horizontalVelocity(0), spline_elevation(0),
            transport_time(0), transport_seat(0), transport_time2(0)
        {
        }

        uint32 transport_time;
        uint32 transport_time2;
        float pitchAngle;
        uint32 fallTime;
        float jump_verticalVelocity;
        float jump_directionX;
        float jump_directionY;
        float jump_horizontalVelocity;
        float spline_elevation;
        int8 transport_seat;
    };

    struct ClientMoveState : public _ClientMoveState
    {
        ClientMoveState()
        {
        }

        Location world_position;
        Location transport_position;
        ObjectGuid transport_guid;
        UnitMoveFlag moveFlags;
        MSTime ms_time;

        std::string ToString() const;
    };

    struct ClientMoveStateChange : public ClientMoveState
    {
        ClientMoveStateChange() : floatValueType(Parameter_End), floatValue(0), allowFlagApply(false) {}

        FloatParameter floatValueType;
        float floatValue;
        UnitMoveFlag allowFlagChange;
        bool allowFlagApply;
    };

    inline std::string ClientMoveState::ToString() const
    {
        std::stringstream st;
        st << "Movement  flags: " << moveFlags.ToString() << std::endl;
        st << "Global position: " << world_position.toString() << std::endl;

        if (moveFlags.ontransport)
        {
            st << "Local  position: " << transport_position.toString() << std::endl;
            st << "seat         Id: " << transport_seat << std::endl;
        }

        if (moveFlags & UnitMoveFlag::Mask_Pitching)
        {
            st << "pitch angle " << pitchAngle << std::endl;
        }

        if (moveFlags.falling)
        {
            st << "jump vertical   vel " << jump_verticalVelocity << std::endl;
            st << "jump direction2d  x " << jump_directionX << std::endl;
            st << "jump direction2d  y " << jump_directionY << std::endl;
            st << "jump horizontal vel " << jump_horizontalVelocity << std::endl;

            st << "fall           time " << fallTime << std::endl;
        }
        return st.str();
    }
}
