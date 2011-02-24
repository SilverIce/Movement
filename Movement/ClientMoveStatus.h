#pragma once

#include "G3D/Vector3.h"
#include "UnitMoveFlags.h"

namespace Movement
{
    struct TransportInfo
    {
        TransportInfo() : t_guid(0), t_time(0), t_seat(0), t_time2(2) {}
        uint64 t_guid;
        Vector3 position;
        float orientation;
        uint32 t_time;
        int8 t_seat;
        uint32 t_time2;
    };

    struct ClientMoveState
    {
        ClientMoveState() : mover(0), ms_time(0), pitch(0), fallTime(0),
            j_velocity(0), j_sinAngle(0), j_cosAngle(0), j_xy_velocy(0), u_unk1(0)
        {
        }

        uint64 mover;
        UnitMoveFlag moveFlags;
        UnitMoveFlag2 moveFlags2;
        uint32 ms_time;
        Vector3 position3;
        float orientation;
        TransportInfo transport;
        float pitch;
        uint32 fallTime;
        float j_velocity;
        float j_sinAngle;
        float j_cosAngle;
        float j_xy_velocy;
        float u_unk1;
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
