#pragma once

#include "typedefs.h"
#include "mov_constants.h"
#include "spline.h"

namespace Movement {

    struct SplineInfo
    {
        // SPLINE_MASK_FINAL_FACING
        union
        {
            struct Point{
                float x,y,z;
            } facing_spot;
            uint64 facing_target;
            float facing_angle;
        };

        uint32          sequience_Id;
        uint32          splineflags;

        // SPLINEFLAG_ANIMATION
        uint8           animationType;
        uint32          animationTime;
    };

    // handles movement by parabolic trajectory
    class ParabolicHandler 
    {
    public:
        ParabolicHandler() {init();}

        float   z_acceleration;
        uint32  parabolic_time_shift;

        void handleParabolic(uint32 t_duration, uint32 t_passed, Vector3& position) const;

    protected:
        void init()
        {
            z_acceleration = 0.f;
            parabolic_time_shift = 0;
        }
    };

    // handles movement by spline 
    class SplineHandler : private SplinePure
    {
    public:
        SplineHandler() {init();}

        using SplinePure::init_path;
        using SplinePure::length;
        using SplinePure::mode;

        uint32  time_passed;
        uint32  duration;
        float   duration_mod;
        float   sync_coeff;

        bool arrived() const
        {
            return time_passed == duration;
        }

        uint32 nodes_count() const
        {
            return points.size();
        }

        const Vector3& getNode(uint32 i) const
        {
            return points[i];
        }

        const PointsArray& getPath() const
        {
            return points;
        }

        void handleSpline(uint32 t_diff, Vector3& position);

    protected:
        void init()
        {
            time_passed  = 0;
            duration     = 0;
            duration_mod = 1.f;
            sync_coeff   = 1.f;
        }
    };

    class MoveSpline : public SplineInfo, public SplineHandler, public ParabolicHandler
    {
        friend class PacketBuilder;
    public:

        //SplinePure          spline;
        Vector3             finalDestination;

    public:

        void AddSplineFlag(uint32 f) { splineflags |= f; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f; }
        bool HasSplineFlag(uint32 f) const { return splineflags & f; }
        uint32 GetSplineFlags() const { return splineflags; }
        void SetSplineFlags(uint32 f) { splineflags = f; }

        void ToggeSplineFlag(uint32 f, bool apply)
        {
            if (apply)
                AddSplineFlag(f);
            else
                RemoveSplineFlag(f);
        }

    public:

        MoveSpline();

        /// facing info
        void SetFacing(uint64 target_guid);
        void SetFacing(float angle);
        void SetFacing(Vector3 const& point);
        void ResetFacing();

        void init_path(const Vector3 * controls, const int count, float curr_speed, float cyclic)
        {
            SplineHandler::init();
            SplineHandler::init_path(controls, count, (SplineMode)HasSplineFlag(SPLINEFLAG_FLYING|SPLINEFLAG_CATMULLROM), cyclic);

            duration = length() / curr_speed * 1000.f;

            // TODO: where this should be handled?
            if (duration == 0)
            {
                movLog.write("Error: null spline duration");
                duration = 1;
            }

            ToggeSplineFlag(SPLINEFLAG_CYCLIC, cyclic);
            if (cyclic)
                finalDestination = Vector3::zero();
            else
                finalDestination = getPath().back();
        }

        void setDone() { AddSplineFlag(SPLINEFLAG_DONE); }
        bool isCyclic() const { return HasSplineFlag(SPLINEFLAG_CYCLIC); }
    };
}

