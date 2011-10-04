/**
  file base:    MoveSplineInitArgs.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      19:2:2011
*/

#pragma once

#include "MoveSplineFlag.h"
#include <G3D/Vector3.h>

namespace Movement
{
    typedef std::vector<Vector3> PointsArray;

    union FacingInfo
    {
        struct{
            float x,y,z;
        };
        uint64  target;
        float   angle;

        FacingInfo(float o) : angle(o) {}
        FacingInfo(uint64 t) : target(t) {}
        FacingInfo() {}
    };

    struct MoveSplineInitArgs
    {
        explicit MoveSplineInitArgs(size_t path_capacity = 16) : path_Idx_offset(0),
            velocity(0.f), parabolic_amplitude(0.f), time_perc(0.f), initialOrientation(0.f), splineId(0)
        {
            path.reserve(path_capacity);
        }
       
        PointsArray path;
        FacingInfo facing;
        MoveSplineFlag flags;
        int32 path_Idx_offset;
        float velocity;
        float parabolic_amplitude;
        float time_perc;
        float initialOrientation;
        uint32 splineId;

        /**	Returns true to show that the arguments were configured correctly and MoveSpline initialization will succeed. */
        bool Validate() const;
    private:
        bool _checkPathBounds() const;
    };
}
