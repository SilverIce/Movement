
/**
  file:         MoveSplineFlag.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once
#include "typedefs.h"

namespace Movement
{
    class MoveSplineFlag
    {
    public:
        enum eFlags{
            None         = 0x00000000,
                                                 // x00-xFF(first byte) used as animation Ids storage in pair with Animation flag
            Done         = 0x00000100,

            Falling      = 0x00000200,           // Affects elevation computation
                                                 // Can't be combined with Parabolic flag
            No_Spline    = 0x00000400,
            Parabolic    = 0x00000800,           // Affects elevation computation
                                                 // Can't be combined with Falling flag
            Walkmode     = 0x00001000,
            Flying       = 0x00002000,           // Smooth movement(Catmullrom interpolation mode), flying animation
            Knockback    = 0x00004000,           // Model orientation fixed
            Final_Point  = 0x00008000,
            Final_Target = 0x00010000,
            Final_Angle  = 0x00020000,
            Catmullrom   = 0x00040000,           // Used Catmullrom interpolation mode
            Cyclic       = 0x00080000,           // Movement by cycled spline
            Enter_Cycle  = 0x00100000,           // Everytimes appears with cyclic flag in monster move packet
            Animation    = 0x00200000,           // Plays animation after some time passed
                                                 // Can't be combined with Parabolic and Falling flags
            Instant      = 0x00400000,           // Finalizes movement, forces unit to arrive to end of the path
            Unknown5     = 0x00800000,
            Unknown6     = 0x01000000,
            Unknown7     = 0x02000000,
            Unknown8     = 0x04000000,
            Backward     = 0x08000000,
            Unknown10    = 0x10000000,
            Unknown11    = 0x20000000,
            Unknown12    = 0x40000000,
            Unknown13    = 0x80000000,

            // Masks
            Mask_Final_Facing = Final_Point | Final_Target | Final_Angle,
            // animation ids stored here, see AnimType enum, used with Animation flag
            Mask_Animations = 0xFF,
            // flags that shouldn't be appended into SMSG_MONSTER_MOVE\SMSG_MONSTER_MOVE_TRANSPORT packet, should be more probably
            Mask_No_Monster_Move = Mask_Final_Facing | Mask_Animations | Done,
            // CatmullRom interpolation mode used
            Mask_CatmullRom = Flying | Catmullrom,
            // Unused, not suported flags
            Mask_Unused = No_Spline|Enter_Cycle|Instant|Unknown5|Unknown6|Unknown7|Unknown8|Unknown10|Unknown11|Unknown12|Unknown13,
        };

        MoveSplineFlag() : raw(0) {}
        MoveSplineFlag(uint32 f) : raw(f) {}
        MoveSplineFlag(const MoveSplineFlag& f) : raw(f.raw) {}

        // Constant interface

        bool isSmooth() const { return raw & Mask_CatmullRom;}
        bool isLinear() const { return !isSmooth();}
        bool isFacing() const { return raw & Mask_Final_Facing;}

        uint8 getAnimationId() const { return animId;}
        bool hasAllFlags(uint32 f) const { return (raw & f) == f;}
        uint32 operator & (uint32 f) const { return (raw & f);}
        uint32 operator | (uint32 f) const { return (raw | f);}

        // Not constant interface

        void operator &= (uint32 f) { raw &= f;}
        void operator |= (uint32 f) { raw |= f;}

        void EnableAnimation(uint8 anim) { raw = raw & ~(Mask_Animations|Falling|Parabolic|Knockback) | Animation|anim;}
        void EnableParabolic() { raw = raw & ~(Mask_Animations|Falling|Animation) | Parabolic;}
        void EnableFalling() { raw = raw & ~(Mask_Animations|Parabolic|Knockback|Animation) | Falling;}
        void EnableFlying() { raw = raw & ~Catmullrom | Flying; }
        void EnableCatmullRom() { raw = raw & ~Flying | Catmullrom; }
        void EnableFacingPoint() { raw = raw & ~Mask_Final_Facing | Final_Point;}
        void EnableFacingAngle() { raw = raw & ~Mask_Final_Facing | Final_Angle;}
        void EnableFacingTarget() { raw = raw & ~Mask_Final_Facing | Final_Target;}

        union
        {
            uint32 raw;

            struct
            {
                uint8 animId       : 8;
                bool done          : 1;
                bool falling       : 1;
                bool no_spline     : 1;

                bool parabolic     : 1;           // affects elevation computation
                                                  // can't be combined with falling flag
                bool walkmode      : 1;
                bool flying        : 1;           // smooth movement(catmullrom interpolation mode) flying animation
                bool knockback     : 1;           // model orientation fixed
                bool final_point   : 1;
                bool final_target  : 1;
                bool final_angle   : 1;
                bool catmullrom    : 1;           // used catmullrom interpolation mode
                bool cyclic        : 1;           // movement by cycled spline
                bool enter_cycle   : 1;           // everytimes appears with cyclic flag in monster move packet
                bool animation     : 1;           // plays animation after some time passed
                                                  // can't be combined with parabolic and falling flags
                bool instant       : 1;           // finalizes movement, forces unit to arrive to end of the path
                bool unknown5      : 1;
                bool unknown6      : 1;
                bool unknown7      : 1;
                bool unknown8      : 1;
                bool backward      : 1;
                bool unknown10     : 1;
                bool unknown11     : 1;
                bool unknown12     : 1;
                bool unknown13     : 1;
            };
        };
    };
}

