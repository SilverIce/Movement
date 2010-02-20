/**
  file base:    MoveSplineInitArgs.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      19:2:2011
*/

#pragma once

#include "MoveSplineFlag.h"

namespace Movement
{
    union FacingInfo
    {
        struct Point{
            float x,y,z;
        }       spot;
        uint64  target;
        float   angle;

        FacingInfo(const Point& p) : spot(p){}
        FacingInfo(float o) : angle(o) {}
        FacingInfo(uint64 t) : target(t) {}
        FacingInfo() {}
    };

    struct MoveSplineInitArgs
    {
        MoveSplineInitArgs() : path_Idx_offset(0),
            velocity(0.f), parabolic_heigth(0.f), time_perc(0.f)   {}
       
        PointsArray path;
        FacingInfo facing;
        MoveSplineFlag flags;
        int32 path_Idx_offset;
        float velocity;
        float parabolic_heigth;
        float time_perc;

        bool Validate() const;
    private:
        bool _checkPathBounds() const;
    };
}
