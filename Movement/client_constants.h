#pragma once

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
        SPLINEFLAG_KNOCKBACK    = 0x00004000,
        SPLINEFLAG_FINALPOINT   = 0x00008000,
        SPLINEFLAG_FINALTARGET  = 0x00010000,
        SPLINEFLAG_FINALFACING  = 0x00020000,
        SPLINEFLAG_CATMULLROM   = 0x00040000,
        SPLINEFLAG_UNKNOWN1     = 0x00080000,
        SPLINEFLAG_UNKNOWN2     = 0x00100000,
        SPLINEFLAG_UNKNOWN3     = 0x00200000,
        SPLINEFLAG_UNKNOWN4     = 0x00400000,
        SPLINEFLAG_UNKNOWN5     = 0x00800000,
        SPLINEFLAG_UNKNOWN6     = 0x01000000,
        SPLINEFLAG_UNKNOWN7     = 0x02000000,
        SPLINEFLAG_UNKNOWN8     = 0x04000000,
        SPLINEFLAG_UNKNOWN9     = 0x08000000,
        SPLINEFLAG_UNKNOWN10    = 0x10000000,
        SPLINEFLAG_UNKNOWN11    = 0x20000000,
        SPLINEFLAG_UNKNOWN12    = 0x40000000,

        // Masks
        SPLINE_MASK_FINAL_FACING = SPLINEFLAG_FINALPOINT | SPLINEFLAG_FINALTARGET | SPLINEFLAG_FINALFACING,
    };

    enum SplineMode
    {
        SPLINEMODE_LINEAR       = 0,
        SPLINEMODE_CATMULLROM   = 1,
        SPLINEMODE_BEZIER3      = 2
    };

    enum SplineType
    {
        SPLINETYPE_NORMAL       = 0,
        SPLINETYPE_STOP         = 1,
        SPLINETYPE_FACINGSPOT   = 2,
        SPLINETYPE_FACINGTARGET = 3,
        SPLINETYPE_FACINGANGLE  = 4
    };

    enum MoveMode
    {
        MoveModeWalk       = 0,
        MoveModeRoot       = 2,
        MoveModeSwim       = 4,
        MoveModeWaterwalk  = 6,
        MoveModeSlow_fall  = 8,
        MoveModeHover      = 10,
        MoveModeFly        = 12,
        //SPLINE_MODE     = 14,
    };

    enum UnitMoveType
    {
        MOVE_NONE           =-1,
        MOVE_WALK           = 0,
        MOVE_RUN            = 1,
        MOVE_SWIM_BACK      = 2,
        MOVE_SWIM           = 3,
        MOVE_RUN_BACK       = 4,
        MOVE_FLIGHT         = 5,
        MOVE_FLIGHT_BACK    = 6,

        MOVE_TURN_RATE      = 7,
        MOVE_PITCH_RATE     = 8,

        MAX_MOVE_TYPE       = 9,
    };

    static const float gravity_rate = 19.291105f;
    static const float terminalVelocity = 60.148003f;

    /*class MoveTypeSelector
    {
    public:
        enum {
            SPEED_MASK = DIRECTIONS_MASK | MOVEFLAG_ROOT | MOVEFLAG_WALK_MODE |
            MOVEFLAG_SWIMMING | MOVEFLAG_FLYING,
        };

        static UnitMoveType Select(const uint32& fl)
        {
            if( (fl & DIRECTIONS_MASK)==NULL || (fl & MOVEFLAG_ROOT))
                return MOVE_NONE;

            if(fl & MOVEFLAG_WALK_MODE)
                return MOVE_WALK;

            if(fl & MOVEFLAG_BACKWARD)
            {
                if(fl & MOVEFLAG_SWIMMING)
                    return MOVE_SWIM_BACK;
                if(fl & MOVEFLAG_FLYING)
                    return MOVE_FLIGHT_BACK;

                return MOVE_RUN_BACK;
            }
            // all directions except backward
            if(fl & (DIRECTIONS_MASK & ~MOVEFLAG_BACKWARD))
            {
                if(fl & MOVEFLAG_SWIMMING)
                    return MOVE_SWIM;
                if(fl & MOVEFLAG_FLYING)
                    return MOVE_FLIGHT;
                return MOVE_RUN;
            }
            return MOVE_NONE;
        }
    };*/
}
