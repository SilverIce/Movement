#pragma once

namespace Movement
{
    static const uint32 Mode2Flag_table[]=
    {
        {/*WALK_BEGAN,*/        MOVEFLAG_WALK_MODE},
        {0},
        {/*ROOT_BEGAN,*/        MOVEFLAG_ROOT},
        {0},   
        {/*SWIM_BEGAN,*/        MOVEFLAG_SWIMMING},
        {0},
        {/*WATERWALK_MODE,*/    MOVEFLAG_WATERWALKING},
        {0},
        {/*SLOW_FALL_BEGAN,*/   MOVEFLAG_SAFE_FALL},
        {0},
        {/*HOVER_BEGAN,*/       MOVEFLAG_HOVER},
        {0},
        {/*FLY_BEGAN, */        MOVEFLAG_FLYING},
        {0},
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

    static const uint16 S_Mode2Opc_table[]=
    {
        {/*WALK_BEGAN,*/        SMSG_SPLINE_MOVE_SET_WALK_MODE},
        {/*WALK_STOP,*/         SMSG_SPLINE_MOVE_SET_RUN_MODE},
        {/*ROOT_BEGAN,*/        SMSG_SPLINE_MOVE_ROOT},
        {/*ROOT_STOP,*/         SMSG_SPLINE_MOVE_UNROOT},   
        {/*SWIM_BEGAN,*/        SMSG_SPLINE_MOVE_START_SWIM},
        {/*SWIM_STOP,*/         SMSG_SPLINE_MOVE_STOP_SWIM},
        {/*WATERWALK_MODE,*/    SMSG_SPLINE_MOVE_WATER_WALK},
        {/*WATERWALK_STOP,*/    SMSG_SPLINE_MOVE_LAND_WALK},
        {/*SLOW_FALL_BEGAN,*/   SMSG_SPLINE_MOVE_FEATHER_FALL},
        {/*SLOW_FALL_END,*/     SMSG_SPLINE_MOVE_NORMAL_FALL},
        {/*HOVER_BEGAN,*/       SMSG_SPLINE_MOVE_SET_HOVER},
        {/*HOVER_STOP,*/        SMSG_SPLINE_MOVE_UNSET_HOVER},
        {/*FLY_BEGAN,*/         SMSG_SPLINE_MOVE_SET_FLYING},
        {/*FLY_STOP,*/          SMSG_SPLINE_MOVE_UNSET_FLYING},
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

}
