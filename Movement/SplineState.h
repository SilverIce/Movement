#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "Node.h"

namespace Movement {

    class SplineState
    {
    public:

        SplineState();

        union FaceData
        {
            struct Point{
                float x,y,z;
            } spot;
            uint64 target;
            float angle;
        };

        FaceData        facing_info;

        // Spline & Movement states has independant timestamp fields
        uint32          time_stamp;
        uint32          splineflags;

        float           total_lenght;
        uint32          move_time_full;
        uint32          move_time_passed;

        float           parabolic_speed;
        uint32          parabolic_time;

        // cached path points
        NodeList        spline_path;

        SplineMode      mode;

        void AddSplineFlag(uint32 f) { splineflags |= f; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f; }
        bool HasSplineFlag(uint32 f) const { return splineflags & f; }
        uint32 GetSplineFlags() const { return splineflags; }
        void SetSplineFlags(uint32 f) { splineflags = f; }

        /// facing info
        void SetFacing(uint64 target_guid);
        void SetFacing(float angle);
        void SetFacing(Vector3 const& point);
        void ResetFacing();

        float TimePassedCoeff() const { return (float(move_time_passed) / float(move_time_full));}
        uint32 TimeElapsed() const { return move_time_full - move_time_passed;}
    };
}

