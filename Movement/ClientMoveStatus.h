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
        Parameter_SpeedCurrent,
        Parameter_End,
    };

    /** Contains unused fields */
    struct _ClientMoveState
    {
        _ClientMoveState() : pitch(0), fallTime(0),
            jump_velocity(0), jump_sinAngle(0), jump_cosAngle(0), jump_fall_velocity(0), spline_elevation(0),
            transport_time(0), transport_seat(0), transport_time2(0)
        {
        }

        uint32 transport_time;
        uint32 transport_time2;
        float pitch;
        uint32 fallTime;
        float jump_velocity;
        float jump_cosAngle;
        float jump_sinAngle;
        float jump_fall_velocity;
        float spline_elevation;
        int8 transport_seat;
    };

    struct ClientMoveState : public _ClientMoveState
    {
        ClientMoveState() : t_guid(0)
        {
        }

        Location world_position;
        Location transport_position;
        uint64 t_guid;
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
            st << "Local  position: " << transport_position.toString() << std::endl;

        if (moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying | UnitMoveFlag::Allow_Pitching))
        {
            st << "pitch angle " << pitch << std::endl;
        }

        if (moveFlags.falling)
        {
            st << "jump z  vel " << jump_velocity << std::endl;
            st << "jump    sin " << jump_sinAngle << std::endl;
            st << "jump    cos " << jump_cosAngle << std::endl;
            st << "jump fall vel " << jump_fall_velocity << std::endl;
        }
        return st.str();
    }
}
