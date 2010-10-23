#pragma once

#include "client_constants.h"
#include "opcodes.h"

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
        //{/*SPLINE_BEGAN, */     MOVEFLAG_SPLINE_ENABLED},
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

    const float absolute_velocy = 7.f;

    const double gravity = 19.29110527038574;

    const double gravity_rate_half = 19.29110527038574/2;

    const float terminalVelocity = 60.148003f;

}
