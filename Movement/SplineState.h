#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "Node.h"
#include "spline_mover.h"

namespace Movement {

    class SplineState : public BaseMover
    {
    public:
        union FaceData
        {
            struct Point{
                float x,y,z;
            } spot;
            uint64 target;
            float angle;
        };
        // this is shoud be store here? its related to movement?  not sure
        FaceData        facing_info;

        // Spline & Movement states has independant timestamp fields
        uint32          splineflags;

        float           parabolic_speed;
        uint32          parabolic_time;

    public:

        SplineState();

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

        void append_path_and_run(const std::vector<Vector3>& path, uint32 ms_time);
    };
}

