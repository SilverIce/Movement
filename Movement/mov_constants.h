#pragma once

#include "typedefs.h"

namespace Movement
{
    enum Directions
    {
        DIRECTION_NONE          = 0x00,
        DIRECTION_FORWARD       = 0x01,
        DIRECTION_BACKWARD      = 0x02,
        DIRECTION_STRAFE_LEFT   = 0x04,
        DIRECTION_STRAFE_RIGHT  = 0x08,
        DIRECTION_TURN_LEFT     = 0x10,
        DIRECTION_TURN_RIGHT    = 0x20,
        DIRECTION_PITCH_UP      = 0x40,
        DIRECTION_PITCH_DOWN    = 0x80,

        DIRECTIONS_MASK          = (DIRECTION_FORWARD | DIRECTION_BACKWARD | DIRECTION_STRAFE_LEFT | DIRECTION_STRAFE_RIGHT |
                                    DIRECTION_TURN_LEFT | DIRECTION_TURN_RIGHT | DIRECTION_PITCH_UP | DIRECTION_PITCH_DOWN),
    };


    // used in most movement packets (send and received)
    enum MovementFlags
    {
        MOVEFLAG_NONE               = 0x00000000,
        MOVEFLAG_FORWARD            = 0x00000001,
        MOVEFLAG_BACKWARD           = 0x00000002,
        MOVEFLAG_STRAFE_LEFT        = 0x00000004,
        MOVEFLAG_STRAFE_RIGHT       = 0x00000008,
        MOVEFLAG_TURN_LEFT          = 0x00000010,
        MOVEFLAG_TURN_RIGHT         = 0x00000020,
        MOVEFLAG_PITCH_UP           = 0x00000040,
        MOVEFLAG_PITCH_DOWN         = 0x00000080,

        MOVEFLAG_WALK_MODE          = 0x00000100,               // Walking
        MOVEFLAG_ONTRANSPORT        = 0x00000200,
        MOVEFLAG_LEVITATING         = 0x00000400,
        MOVEFLAG_ROOT               = 0x00000800,
        MOVEFLAG_FALLING            = 0x00001000,
        MOVEFLAG_FALLINGFAR         = 0x00002000,
        MOVEFLAG_PENDINGSTOP        = 0x00004000,
        MOVEFLAG_PENDINGSTRAFESTOP  = 0x00008000,
        MOVEFLAG_PENDINGFORWARD     = 0x00010000,
        MOVEFLAG_PENDINGBACKWARD    = 0x00020000,
        MOVEFLAG_PENDINGSTRAFELEFT  = 0x00040000,
        MOVEFLAG_PENDINGSTRAFERIGHT = 0x00080000,
        MOVEFLAG_PENDINGROOT        = 0x00100000,
        MOVEFLAG_SWIMMING           = 0x00200000,               // appears with fly flag also
        MOVEFLAG_ASCENDING          = 0x00400000,               // swim up also
        MOVEFLAG_DESCENDING         = 0x00800000,               // swim down also
        MOVEFLAG_CAN_FLY            = 0x01000000,               // can fly in 3.3?
        MOVEFLAG_FLYING             = 0x02000000,               // Actual flying mode
        MOVEFLAG_SPLINE_ELEVATION   = 0x04000000,               // used for flight paths
        MOVEFLAG_SPLINE_ENABLED     = 0x08000000,               // used for flight paths
        MOVEFLAG_WATERWALKING       = 0x10000000,               // prevent unit from falling through water
        MOVEFLAG_SAFE_FALL          = 0x20000000,               // active rogue safe fall spell (passive)
        MOVEFLAG_HOVER              = 0x40000000
    };

    enum MovementFlags2
    {
        MOVEFLAG2_NONE              = 0x0000,
        MOVEFLAG2_UNK1              = 0x0001,
        MOVEFLAG2_UNK2              = 0x0002,
        MOVEFLAG2_UNK3              = 0x0004,
        MOVEFLAG2_FULLSPEEDTURNING  = 0x0008,
        MOVEFLAG2_FULLSPEEDPITCHING = 0x0010,
        MOVEFLAG2_ALLOW_PITCHING    = 0x0020,
        MOVEFLAG2_UNK4              = 0x0040,
        MOVEFLAG2_UNK5              = 0x0080,
        MOVEFLAG2_UNK6              = 0x0100,
        MOVEFLAG2_UNK7              = 0x0200,
        MOVEFLAG2_INTERP_MOVE       = 0x0400,
        MOVEFLAG2_INTERP_TURNING    = 0x0800,
        MOVEFLAG2_INTERP_PITCHING   = 0x1000,
        MOVEFLAG2_UNK8              = 0x2000,
        MOVEFLAG2_UNK9              = 0x4000,
        MOVEFLAG2_UNK10             = 0x8000,
        MOVEFLAG2_INTERP_MASK       = MOVEFLAG2_INTERP_MOVE | MOVEFLAG2_INTERP_TURNING | MOVEFLAG2_INTERP_PITCHING
    };

    enum SplineFlags
    {
        SPLINEFLAG_NONE         = 0x00000000,
        SPLINEFLAG_FORWARD      = 0x00000001,
        SPLINEFLAG_BACKWARD     = 0x00000002,
        SPLINEFLAG_STRAFE_LEFT  = 0x00000004,
        SPLINEFLAG_STRAFE_RIGHT = 0x00000008,
        SPLINEFLAG_LEFT         = 0x00000010,
        SPLINEFLAG_RIGHT        = 0x00000020,
        SPLINEFLAG_PITCH_UP     = 0x00000040,
        SPLINEFLAG_PITCH_DOWN   = 0x00000080,
        SPLINEFLAG_DONE         = 0x00000100,
        SPLINEFLAG_FALLING      = 0x00000200,
        SPLINEFLAG_NO_SPLINE    = 0x00000400,
        SPLINEFLAG_TRAJECTORY   = 0x00000800,
        SPLINEFLAG_WALKMODE     = 0x00001000,
        SPLINEFLAG_FLYING       = 0x00002000,
        SPLINEFLAG_KNOCKBACK    = 0x00004000,           // model orientation fixed
        SPLINEFLAG_FINAL_POINT   = 0x00008000,
        SPLINEFLAG_FINAL_TARGET  = 0x00010000,
        SPLINEFLAG_FINAL_ANGLE  = 0x00020000,
        SPLINEFLAG_CATMULLROM   = 0x00040000,           // used CatmullRom interpolation mode
        SPLINEFLAG_CYCLIC       = 0x00080000,           // movement by cycled spline 
        SPLINEFLAG_UNKNOWN2     = 0x00100000,           // appears with bezier3, cyclic flags, nodes > 1
        SPLINEFLAG_ANIMATION    = 0x00200000,           // animationId (0...3), uint32 time
        SPLINEFLAG_UNKNOWN4     = 0x00400000,
        SPLINEFLAG_UNKNOWN5     = 0x00800000,
        SPLINEFLAG_UNKNOWN6     = 0x01000000,
        SPLINEFLAG_UNKNOWN7     = 0x02000000,
        SPLINEFLAG_UNKNOWN8     = 0x04000000,
        SPLINEFLAG_UNKNOWN9     = 0x08000000,           // appears with walkmode flag, nodes = 1
        SPLINEFLAG_UNKNOWN10    = 0x10000000,
        SPLINEFLAG_UNKNOWN11    = 0x20000000,
        SPLINEFLAG_UNKNOWN12    = 0x40000000,

        // Masks
        SPLINE_MASK_FINAL_FACING = SPLINEFLAG_FINAL_POINT | SPLINEFLAG_FINAL_TARGET | SPLINEFLAG_FINAL_ANGLE,
        // flags that shouldn't be appended into SMSG_MONSTER_MOVE\SMSG_MONSTER_MOVE_TRANSPORT packet, should be more probably
        SPLINE_MASK_NO_MONSTER_MOVE = SPLINE_MASK_FINAL_FACING | DIRECTIONS_MASK | SPLINEFLAG_DONE,
    };

    enum SplineType
    {
        SPLINETYPE_NORMAL       = 0,
        SPLINETYPE_STOP         = 1,
        SPLINETYPE_FACINGSPOT   = 2,
        SPLINETYPE_FACINGTARGET = 3,
        SPLINETYPE_FACINGANGLE  = 4
    };

    // used with 0x00200000 flag (SPLINEFLAG_ANIMATION) in monster move packet
    enum AnimType
    {
        UNK0 = 0, // 460 = ToGround, index of AnimationData.dbc
        UNK1 = 1, // 461 = FlyToFly?
        UNK2 = 2, // 458 = ToFly
        UNK3 = 3, // 463 = FlyToGround
    };

    enum MoveMode
    {
        MoveModeWalk,
        MoveModeRoot,
        MoveModeSwim,
        MoveModeWaterwalk,
        MoveModeSlowfall,
        MoveModeHover,
        MoveModeFly,
        //SPLINE_MODE      = 7,
        MoveModeMaxCount
    };

    enum SpeedType
    {
        SpeedNone           =-1,

        SpeedCurrent        = 0,
        SpeedWalk           = 1,
        SpeedRun            = 2,
        SpeedSwimBack       = 3,
        SpeedSwim           = 4,
        SpeedRunBack        = 5,
        SpeedFlight         = 6,
        SpeedFlightBack     = 7,

        SpeedTurn           = 8,
        SpeedPitch          = 9,

        SpeedMaxCount       = 10,
    };

    extern const float absolute_velocy;
    extern const double gravity;
    extern const float terminalVelocity;

    extern const uint32 Mode2Flag_table[];
    extern const uint16 S_Speed2Opc_table[];
    extern const uint16 S_Mode2Opc_table[MoveModeMaxCount][2];
    extern const uint16 SetSpeed2Opc_table[][2];
    extern const float  BaseSpeed[SpeedMaxCount];
}
