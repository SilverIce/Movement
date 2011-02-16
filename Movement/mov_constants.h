
/**
  file:         mov_constants.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

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

    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
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
        MoveModeLevitation,
        MoveModeMaxCount
    };

    enum SpeedType
    {
        SpeedNotStandart    =-1,

        SpeedWalk           = 0,
        SpeedRun            = 1,
        SpeedSwimBack       = 2,
        SpeedSwim           = 3,
        SpeedRunBack        = 4,
        SpeedFlight         = 5,
        SpeedFlightBack     = 6,
        SpeedTurn           = 7,
        SpeedPitch          = 8,

        SpeedMaxCount       = 9,
    };

    enum MovControlType
    {
        MovControlClient,
        MovControlServer,
        MovControlCount,
    };

    extern const double gravity;
    extern const float terminalVelocity;

    extern const uint32 Mode2Flag_table[];
    extern const uint16 S_Speed2Opc_table[];
    extern const uint16 S_Mode2Opc_table[MoveModeMaxCount][2];
    extern const uint16 SetSpeed2Opc_table[][2];
    extern const float  BaseSpeed[SpeedMaxCount];

    float computeFallTime(float path_length, bool isSafeFall);
    float computeFallElevation(float t_passed, bool isSafeFall, float start_velocy);
}
