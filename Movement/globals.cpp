#pragma once

#include "mov_constants.h"
#include "opcodes.h"
#include <math.h>

namespace Movement
{
    static const uint32 Mode2Flag_table[]=
    {
        {/*WALK_BEGAN,*/        MOVEFLAG_WALK_MODE},
        {/*ROOT_BEGAN,*/        MOVEFLAG_ROOT},
        {/*SWIM_BEGAN,*/        MOVEFLAG_SWIMMING},
        {/*WATERWALK_MODE,*/    MOVEFLAG_WATERWALKING},
        {/*SLOW_FALL_BEGAN,*/   MOVEFLAG_SAFE_FALL},
        {/*HOVER_BEGAN,*/       MOVEFLAG_HOVER},
        {/*FLY_BEGAN, */        MOVEFLAG_FLYING},
        {                       MOVEFLAG_LEVITATING},
        //{0},
    };

    static const uint16 S_Speed2Opc_table[]=
    {
        {/*MOVE_WALK,*/         SMSG_SPLINE_SET_WALK_SPEED},
        {/*MOVE_RUN,*/          SMSG_SPLINE_SET_RUN_SPEED},
        {/*MOVE_SWIM_BACK,*/    SMSG_SPLINE_SET_SWIM_BACK_SPEED},
        {/*MOVE_SWIM,*/         SMSG_SPLINE_SET_SWIM_SPEED},
        {/*MOVE_RUN_BACK,*/     SMSG_SPLINE_SET_RUN_BACK_SPEED},
        {/*MOVE_FLIGHT,*/       SMSG_SPLINE_SET_FLIGHT_SPEED},
        {/*MOVE_FLIGHT_BACK,*/  SMSG_SPLINE_SET_FLIGHT_BACK_SPEED},
        {/*MOVE_TURN_RATE,*/    SMSG_SPLINE_SET_TURN_RATE},
        {/*MOVE_PITCH_RATE,*/   SMSG_SPLINE_SET_PITCH_RATE},
    };

    static const uint16 S_Mode2Opc_table[MoveModeMaxCount][2]=
    {
        {/*WALK_BEGAN,*/        SMSG_SPLINE_MOVE_SET_RUN_MODE, SMSG_SPLINE_MOVE_SET_WALK_MODE},
        {/*ROOT_BEGAN,*/        SMSG_SPLINE_MOVE_UNROOT,       SMSG_SPLINE_MOVE_ROOT},
        {/*SWIM_BEGAN,*/        SMSG_SPLINE_MOVE_STOP_SWIM,    SMSG_SPLINE_MOVE_START_SWIM},
        {/*WATERWALK_MODE,*/    SMSG_SPLINE_MOVE_LAND_WALK,    SMSG_SPLINE_MOVE_WATER_WALK},
        {/*SLOW_FALL_BEGAN,*/   SMSG_SPLINE_MOVE_NORMAL_FALL,  SMSG_SPLINE_MOVE_FEATHER_FALL},
        {/*HOVER_BEGAN,*/       SMSG_SPLINE_MOVE_UNSET_HOVER,  SMSG_SPLINE_MOVE_SET_HOVER},
        {/*FLY_BEGAN,*/         SMSG_SPLINE_MOVE_UNSET_FLYING, SMSG_SPLINE_MOVE_SET_FLYING},
        {0,0},
    };

    static const uint16 SetSpeed2Opc_table[][2]=
    {
        {MSG_MOVE_SET_WALK_SPEED,       SMSG_FORCE_WALK_SPEED_CHANGE},
        {MSG_MOVE_SET_RUN_SPEED,        SMSG_FORCE_RUN_SPEED_CHANGE},
        {MSG_MOVE_SET_SWIM_BACK_SPEED,  SMSG_FORCE_SWIM_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_SWIM_SPEED,       SMSG_FORCE_SWIM_SPEED_CHANGE},
        {MSG_MOVE_SET_RUN_BACK_SPEED,   SMSG_FORCE_RUN_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_FLIGHT_SPEED,     SMSG_FORCE_FLIGHT_SPEED_CHANGE},
        {MSG_MOVE_SET_FLIGHT_BACK_SPEED,SMSG_FORCE_FLIGHT_BACK_SPEED_CHANGE},
        {MSG_MOVE_SET_TURN_RATE,        SMSG_FORCE_TURN_RATE_CHANGE},
        {MSG_MOVE_SET_PITCH_RATE,       SMSG_FORCE_PITCH_RATE_CHANGE},
    };

    static const float BaseSpeed[SpeedMaxCount] =
    {
        7.0f,                                                   // SpeedCurrent
        2.5f,                                                   // SpeedWalk
        7.0f,                                                   // SpeedRun
        4.5f,                                                   // SpeedSwimBack
        4.722222f,                                              // SpeedSwim
        1.25f,                                                  // SpeedRunBack
        7.0f,                                                   // SpeedFlight
        4.5f,                                                   // SpeedFlightBack
        3.141594f,                                              // SpeedTurn
        3.141594f,                                              // SpeedPitch
    };

    const double gravity = 19.29110527038574;

    const float terminalVelocity = 60.148003f;

    const float savefallVelocity = 7.f;

    uint32 computeFallTime(float path_length, bool isSafeFall)
    {
        static const float terminal_length = (terminalVelocity * terminalVelocity) / (2.f * gravity);
        static const float safe_terminal_length = (savefallVelocity * savefallVelocity) / (2.f * gravity);

        float time;
        if ( isSafeFall )
        {
            if (path_length >= safe_terminal_length)
                time = (path_length - safe_terminal_length)/savefallVelocity + savefallVelocity/gravity;
            else
                time = sqrtf(2.f * path_length/gravity);
        }
        else
        {
            if (path_length >= terminal_length)
                time = (path_length - terminal_length)/terminalVelocity + terminalVelocity/gravity;
            else
                time = sqrtf(2.f * path_length/gravity);
        }

        return uint32(time * 1000.f);
    }

    float computeFallElevation( float t_passed, bool isSafeFall, float start_velocy )
    {
        float termVel;
        float result;

        if ( isSafeFall )
            termVel = savefallVelocity;
        else
            termVel = terminalVelocity;

        if ( start_velocy > termVel )
            start_velocy = termVel;

        if ( gravity * t_passed + start_velocy > termVel )
        {
            float dvel = (termVel - start_velocy);
            result = termVel * (t_passed - dvel/gravity) + dvel*(start_velocy + dvel / 2.f) / gravity;
        }
        else
            result = t_passed * (start_velocy + t_passed * gravity / 2.f);

        return result;
    }
}
