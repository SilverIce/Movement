#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "spline.h"

namespace Movement {

    class SplineState
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

        uint32          splineflags;

        float           parabolic_speed;
        uint32          parabolic_time;

        // Spline & Movement states have independant timestamp fields
        uint32          last_ms_time;
        uint32          time_passed;
        uint32          last_positionIdx;

        SplinePure      spline;

        float           passed_length;


    private:

        void AddSplineFlag(uint32 f) { splineflags |= f; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f; }
        bool HasSplineFlag(uint32 f) const { return splineflags & f; }
        uint32 GetSplineFlags() const { return splineflags; }
        void SetSplineFlags(uint32 f) { splineflags = f; }

    public:

        SplineState();

        /// facing info
        void SetFacing(uint64 target_guid);
        void SetFacing(float angle);
        void SetFacing(Vector3 const& point);
        void ResetFacing();

        void init_path(const Vector3 * controls, const int count, SplineMode m, bool cyclic);
        void UpdatePosition(uint32 curr_ms_time, float velocy, Vector3 & c);
        uint32 duration() const { return spline.duration(); }
    };
}

