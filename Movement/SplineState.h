#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "Node.h"

namespace Movement {

    class SplineState
    {
    public:

        SplineState()
        {
            spline_path.reserve(10);
        }

        union FaceData
        {
            struct Point 
            {
                float x,y,z;
            } spot;
            
            uint64 target;
            float angle;
        } facing_info;

        // Spline & Movement states have independant timestamp fields
        uint32 time_stamp;

        uint32 splineflags;

        float total_lenght;
        uint32 move_time_full;
        uint32 move_time_passed;

        // cached path points
        NodeList spline_path;

        void AddSplineFlag(uint32 f) { splineflags |= f; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f; }
        bool HasSplineFlag(uint32 f) const { return splineflags & f; }
        uint32 GetSplineFlags() const { return splineflags; }
        void SetSplineFlags(uint32 f) { splineflags = f; }

        /// Get-Set, facing info
        void SetFacing(uint64 guid);
        void SetFacing(float o);
        void SetFacing(Vector3 const& spot);

        void ResetFacing();

        float TimePassedCoeff() const { return (float(move_time_passed) / float(move_time_full));}
        uint32 TimeElapsed() const { return move_time_full - move_time_passed;}
    };
}

