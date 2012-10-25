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

    inline FloatParameter SelectSpeedType(UnitMoveFlag moveFlags)
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

        Vector4 globalPosition;
        Vector4 relativePosition;
        ObjectGuid transport_guid;
        UnitMoveFlag moveFlags;
        MSTime ms_time;
        float pitchAngle;
        int8 transport_seat;

        QString toString() const;
    };

    struct ClientMoveStateChange : public ClientMoveState
    {
        ClientMoveStateChange() : floatValueType(Parameter_End), floatValue(0), allowFlagApply(false) {}

        FloatParameter floatValueType;
        float floatValue;
        UnitMoveFlag allowFlagChange;
        bool allowFlagApply;
    };

    inline QString ClientMoveState::toString() const
    {
        QString string;
        QTextStream st(&string);
        st << "Movement flags       " << moveFlags.toString() << endl;
        st << "Global position      " << globalPosition.toString().c_str() << endl;

        if (moveFlags.ontransport)
        {
            st << "Local position       " << relativePosition.toString().c_str() << endl;
            st << "seat Id              " << (int32)transport_seat << endl;
        }

        if (moveFlags & UnitMoveFlag::Mask_Pitching)
        {
            st << "pitch angle          " << pitchAngle << endl;
        }

        if (moveFlags.falling)
        {
            st << "jump vertical vel    " << jump_verticalVelocity << endl;
            st << "jump direction2d x   " << jump_directionX << endl;
            st << "jump direction2d y   " << jump_directionY << endl;
            st << "jump horizontal vel  " << jump_horizontalVelocity << endl;
            st << "fall time            " << fallTime << endl;
        }

        if (moveFlags.spline_elevation)
        {
            st << "spline elevation     " << spline_elevation << endl;
        }
        return *st.string();
    }
}
