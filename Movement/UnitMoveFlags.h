/**
  file base:    UnitMoveFlags.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once

#include "typedefs.h"

namespace Movement
{
    class UnitMoveFlag
    {
    public:
        enum eUnitMoveFlags
        {
            None               = 0x00000000,
            Forward            = 0x00000001,
            Backward           = 0x00000002,
            Strafe_Left        = 0x00000004,
            Strafe_Right       = 0x00000008,
            Turn_Left          = 0x00000010,
            Turn_Right         = 0x00000020,
            Pitch_Up           = 0x00000040,
            Pitch_Down         = 0x00000080,

            Walk_Mode          = 0x00000100,               // Walking
            Ontransport        = 0x00000200,
            Levitating         = 0x00000400,
            Root               = 0x00000800,
            Falling            = 0x00001000,
            Fallingfar         = 0x00002000,
            Pendingstop        = 0x00004000,
            Pendingstrafestop  = 0x00008000,
            Pendingforward     = 0x00010000,
            Pendingbackward    = 0x00020000,
            Pendingstrafeleft  = 0x00040000,
            Pendingstraferight = 0x00080000,
            Pendingroot        = 0x00100000,
            Swimming           = 0x00200000,               // Appears With Fly Flag Also
            Ascending          = 0x00400000,               // Swim Up Also
            Descending         = 0x00800000,               // Swim Down Also
            Can_Fly            = 0x01000000,               // Can Fly In 3.3?
            Flying             = 0x02000000,               // Actual Flying Mode
            Spline_Elevation   = 0x04000000,               // Used For Flight Paths
            Spline_Enabled     = 0x08000000,               // Used For Flight Paths
            Waterwalking       = 0x10000000,               // Prevent Unit From Falling Through Water
            Safe_Fall          = 0x20000000,               // Active Rogue Safe Fall Spell (Passive)
            Hover              = 0x40000000,
            Flag_x80000000     = 0x80000000,

            Mask_Directions = Forward | Backward | Strafe_Left | Strafe_Right | Turn_Left | Turn_Right | Pitch_Up | Pitch_Down,
        };

        UnitMoveFlag() : raw(0) {}
        UnitMoveFlag(uint32 f) : raw(f) {}
        UnitMoveFlag(const UnitMoveFlag& f) : raw(f.raw) {}

        // Constant interface

        bool hasDirection() const { return raw & Mask_Directions;}

        uint32 operator & (uint32 f) const { return (raw & f);}
        uint32 operator | (uint32 f) const { return (raw | f);}

        // Not constant interface

        void operator &= (uint32 f) { raw &= f;}
        void operator |= (uint32 f) { raw |= f;}

        union
        {
            uint32 raw;

            struct
            {
                bool forward             : 1;
                bool backward            : 1;
                bool strafe_left         : 1;
                bool strafe_right        : 1;
                bool turn_left           : 1;
                bool turn_right          : 1;
                bool pitch_up            : 1;
                bool pitch_down          : 1;
                bool walk_mode           : 1;
                bool ontransport         : 1;
                bool levitating          : 1;
                bool root                : 1;
                bool falling             : 1;
                bool fallingfar          : 1;
                bool pendingstop         : 1;
                bool pendingstrafestop   : 1;
                bool pendingforward      : 1;
                bool pendingbackward     : 1;
                bool pendingstrafeleft   : 1;
                bool pendingstraferight  : 1;
                bool pendingroot         : 1;
                bool swimming            : 1;
                bool ascending           : 1;
                bool descending          : 1;
                bool can_fly             : 1;
                bool flying              : 1;
                bool spline_elevation    : 1;
                bool spline_enabled      : 1;
                bool waterwalking        : 1;
                bool safe_fall           : 1;
                bool hover               : 1;
                bool flag_x80000000      : 1;
            };
        };
    };

    class UnitMoveFlag2
    {
    public:
        enum eUnitMoveFlags2
        {
            None              = 0x0000,
            Unk1              = 0x0001,
            Unk2              = 0x0002,
            Unk3              = 0x0004,
            Fullspeedturning  = 0x0008,
            Fullspeedpitching = 0x0010,
            Allow_Pitching    = 0x0020,
            Unk4              = 0x0040,
            Unk5              = 0x0080,
            Unk6              = 0x0100,
            Unk7              = 0x0200,
            Interp_Move       = 0x0400,
            Interp_Turning    = 0x0800,
            Interp_Pitching   = 0x1000,
            Unk8              = 0x2000,
            Unk9              = 0x4000,
            Unk10             = 0x8000,
            Mask_Interp       = Interp_Move | Interp_Turning | Interp_Pitching
        };

        UnitMoveFlag2() : raw(0) {}
        UnitMoveFlag2(uint16 f) : raw(f) {}
        UnitMoveFlag2(const UnitMoveFlag2& f) : raw(f.raw) {}

        // Constant interface

        uint32 operator & (uint32 f) const { return (raw & f);}
        uint32 operator | (uint32 f) const { return (raw | f);}

        // Not constant interface

        void operator &= (uint32 f) { raw &= f;}
        void operator |= (uint32 f) { raw |= f;}


        union
        {
            uint16 raw;

            struct 
            {
                bool unk1              : 1;
                bool unk2              : 1;
                bool unk3              : 1;
                bool fullspeedturning  : 1;
                bool fullspeedpitching : 1;
                bool allow_pitching    : 1;
                bool unk4              : 1;
                bool unk5              : 1;
                bool unk6              : 1;
                bool unk7              : 1;
                bool interp_move       : 1;
                bool interp_turning    : 1;
                bool interp_pitching   : 1;
                bool unk8              : 1;
                bool unk9              : 1;
                bool unk10             : 1;
            };
        };
    };
}
