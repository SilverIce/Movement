/**
  file base:    UnitMoveFlags.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      20:2:2011
*/

#pragma once

#include "typedefs.h"
#include <string>

namespace Movement
{
    class UnitMoveFlag
    {
    public:
        typedef uint64 eUnitMoveFlags;

        static const eUnitMoveFlags

            None               = 0x00000000,
            Forward            = 0x00000001,
            Backward           = 0x00000002,
            Strafe_Left        = 0x00000004,
            Strafe_Right       = 0x00000008,
            Turn_Left          = 0x00000010,
            Turn_Right         = 0x00000020,
            Pitch_Up           = 0x00000040,
            Pitch_Down         = 0x00000080,

            Walk_Mode          = 0x00000100,
            Ontransport        = 0x00000200,
            GravityDisabled    = 0x00000400,
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
            Can_Safe_Fall      = 0x20000000,               // Active Rogue Safe Fall Spell (Passive)
            Hover              = 0x40000000,
            Flag_0x80000000    = 0x80000000,

            Unk1               = 0x000100000000L,
            Unk2               = 0x000200000000L,
            Unk3               = 0x000400000000L,
            Fullspeedturning   = 0x000800000000L,
            Fullspeedpitching  = 0x001000000000L,
            Allow_Pitching     = 0x002000000000L,
            Unk4               = 0x004000000000L,
            Unk5               = 0x008000000000L,
            Unk6               = 0x010000000000L,
            Unk7               = 0x020000000000L,
            Interp_Move        = 0x040000000000L,
            Interp_Turning     = 0x080000000000L,
            Interp_Pitching    = 0x100000000000L,
            Unk8               = 0x200000000000L,
            AllowSwimFlyTransition = 0x400000000000L,
            Unk10              = 0x800000000000L,

            Mask_Speed      = Backward | Flying | Walk_Mode | Swimming,
            Mask_Interp     = Interp_Move | Interp_Turning | Interp_Pitching,
            Mask_Directions = Forward | Backward | Strafe_Left | Strafe_Right | Turn_Left | Turn_Right,
            Mask_Moving     = Forward | Backward | Strafe_Left | Strafe_Right | Turn_Left | Turn_Right | Falling | Fallingfar | Ascending | Descending
        ;

        enum {
            Size = sizeof(uint32) + sizeof(uint16),
        };

        UnitMoveFlag() : raw(0) {}
        UnitMoveFlag(uint64 f) : raw(f) {}
        UnitMoveFlag(const UnitMoveFlag& f) : raw(f.raw) {}

        // Constant interface

        bool hasFlag(uint64 f) const { return (raw & f) != 0;}
        bool hasDirection() const { return hasFlag(Mask_Directions);}

        uint64 operator & (uint64 f) const { return (raw & f);}
        uint64 operator | (uint64 f) const { return (raw | f);}
        std::string ToString() const;

        // Not constant interface

        void operator &= (uint64 f) { raw &= f;}
        void operator |= (uint64 f) { raw |= f;}

        union
        {
            uint64 raw;

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
                bool gravity_disabled    : 1;
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
                bool can_safe_fall       : 1;
                bool hover               : 1;
                bool flag_0x80000000     : 1;

                bool unk1                : 1;
                bool unk2                : 1;
                bool unk3                : 1;
                bool fullspeedturning    : 1;
                bool fullspeedpitching   : 1;
                bool allow_pitching      : 1;
                bool unk4                : 1;
                bool unk5                : 1;
                bool unk6                : 1;
                bool unk7                : 1;
                bool interp_move         : 1;
                bool interp_turning      : 1;
                bool interp_pitching     : 1;
                bool unk8                : 1;
                bool allowSwimFlyTransition : 1;
                bool unk10               : 1;
                uint16 unused            : 16;
            };
        };
    };
}
