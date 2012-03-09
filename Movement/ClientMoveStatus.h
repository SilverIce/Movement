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
        Parameter_SpeedCustom,
        Parameter_End,
    };

    /** Contains unused fields */
    struct _ClientMoveState
    {
        _ClientMoveState() {
            transport_time = 0;
            transport_time2 = 0;
            fallTime = 0;
            jump_verticalVelocity = 0;
            jump_directionX = 0;
            jump_directionY = 0;
            jump_horizontalVelocity = 0;
            spline_elevation = 0;
        }

        uint32 transport_time;
        uint32 transport_time2;
        uint32 fallTime;
        float jump_verticalVelocity;
        float jump_directionX;
        float jump_directionY;
        float jump_horizontalVelocity;
        float spline_elevation;
    };

    struct ClientMoveState : public _ClientMoveState
    {
        ClientMoveState() {
            pitchAngle = 0;
            transport_seat = 0;
        }

        Location globalPosition;
        Location relativePosition;
        ObjectGuid transport_guid;
        UnitMoveFlag moveFlags;
        MSTime ms_time;
        float pitchAngle;
        int8 transport_seat;

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
        st << "Global position: " << globalPosition.ToString() << std::endl;

        if (moveFlags.ontransport)
        {
            st << "Local  position: " << relativePosition.ToString() << std::endl;
            st << "seat         Id: " << (int32)transport_seat << std::endl;
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
