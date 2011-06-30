#pragma once

#include "Location.h"
#include "UnitMoveFlags.h"

namespace Movement
{
    /** Contains unused fields */
    struct _ClientMoveState
    {
        _ClientMoveState() : pitch(0), fallTime(0),
            jump_velocity(0), jump_sinAngle(0), jump_cosAngle(0), jump_xy_velocy(0), spline_elevation(0),
            transport_time(0), transport_seat(0), transport_time2(0)
        {
        }

        uint32 transport_time;
        uint32 transport_time2;
        float pitch;
        uint32 fallTime;
        float jump_velocity;
        float jump_sinAngle;
        float jump_cosAngle;
        float jump_xy_velocy;
        float spline_elevation;
        int8 transport_seat;
    };

    struct ClientMoveState : public _ClientMoveState
    {
        ClientMoveState() : /*mover(0),*/ t_guid(0)
        {
        }

        //uint64 mover;
        Location world_position;
        Location transport_position;
        uint64 t_guid;
        UnitMoveFlag moveFlags;
        MSTime ms_time;
    };

    struct ClientMoveEvent
    {
        ClientMoveEvent(uint16 opc) : opcode(opc)
        {
        }

        ClientMoveState state;
        uint16 opcode;
    };
}
