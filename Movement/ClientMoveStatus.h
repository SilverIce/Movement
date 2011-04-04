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

        UnitMoveFlag2 moveFlags2;
        uint32 transport_time;
        int8 transport_seat;
        uint32 transport_time2;
        float pitch;
        uint32 fallTime;
        float jump_velocity;
        float jump_sinAngle;
        float jump_cosAngle;
        float jump_xy_velocy;
        float spline_elevation;
    };

    struct ClientMoveState : public _ClientMoveState
    {
        ClientMoveState() : /*mover(0),*/ ms_time(0), t_guid(0)
        {
        }

        //uint64 mover;
        UnitMoveFlag moveFlags;
        uint32 ms_time;
        Location world_position;
        uint64 t_guid;
        Location transport_position;
    };

    struct ClientMoveEvent
    {
        ClientMoveEvent(uint16 opc) : opcode(opc)
        {
        }

        uint16 opcode;
        ClientMoveState state;
    };
}
