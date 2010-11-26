#pragma once

#include "mov_constants.h"
#include "spline.h"
#include "G3D/Vector4.h"

#include <limits>

namespace Movement {

    template<class T>
    class counter
    {
    public:
        counter() : m_counter(0) {}

        T increase()
        {
            if (m_counter == std::numeric_limits<T>::max())
                m_counter = 1;
            else
                ++m_counter;
            return m_counter;
        }

    private:
        T m_counter;
    };

    // handles movement by parabolic trajectory
    class ParabolicHandler 
    {
    public:
        ParabolicHandler() {init();}

        float   z_acceleration;
        uint32  time_shift;

        void handleParabolic(uint32 t_duration, uint32 t_passed, Vector3& position) const;

        void init()
        {
            z_acceleration = 0.f;
            time_shift = 0;
        }
    };

    class MoveSpline
    {
        friend class PacketBuilder;
    public:

        uint32          sequence_Id;
        uint32          splineflags;

        SplineLive      spline;

        uint32          start_move_time;
        uint32          time_passed;
        uint32          duration;
        float           duration_mod;
        float           sync_coeff;

        Vector3         finalDestination;

        // SPLINEFLAG_TRAJECTORY
        ParabolicHandler parabolic;

        // SPLINE_MASK_FINAL_FACING
        union
        {
            struct Point{
                float x,y,z;
            } facing_spot;
            uint64 facing_target;
            float facing_angle;
        };

        // SPLINEFLAG_ANIMATION
        uint8           animationType;
        uint32          animationTime;

        void Init()
        {
            splineflags = 0;

            animationType = 0;
            animationTime = 0;

            spline.clear();

            start_move_time = 0;
            time_passed  = 0;
            duration     = 0;
            duration_mod = 1.f;
            sync_coeff   = 1.f;

            parabolic.init();
        }

    public:

        void AddSplineFlag(uint32 f) { splineflags |= f; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f; }
        bool HasSplineFlag(uint32 f) const { return splineflags & f; }
        uint32 GetSplineFlags() const { return splineflags; }
        void SetSplineFlags(uint32 f) { splineflags = f; }

        void ApplySplineFlag(uint32 f, bool apply)
        {
            if (apply)
                AddSplineFlag(f);
            else
                RemoveSplineFlag(f);
        }

    public:

        explicit MoveSpline() {Init();}

        /// facing info
        void SetFacing(uint64 target_guid);
        void SetFacing(float angle);
        void SetFacing(Vector3 const& point);
        void ResetFacing();

        void init_spline(uint32 StartMoveTime, PointsArray& path, float velocy, uint32 flags);
        void init_spline(uint32 StartMoveTime, PointsArray& path, uint32 duration, uint32 flags);

        void init_cyclic_spline(uint32 StartMoveTime, PointsArray& path, float velocy, uint32 flags);

        void reset_state()
        {
            Init();
            SetSplineFlags(SPLINEFLAG_WALKMODE);
        }

        void updateState( uint32 ms_time, Vector4& c );

        // helpers
        void setDone() { AddSplineFlag(SPLINEFLAG_DONE); }
        bool isDone() const { return splineflags & SPLINEFLAG_DONE; }

        bool isCyclic() const { return splineflags & SPLINEFLAG_CYCLIC;}
        bool isSmooth() const { return splineflags & (SPLINEFLAG_FLYING|SPLINEFLAG_CATMULLROM);}

        const Vector3& getNode(uint32 i) const { return spline.points[i];}
        const PointsArray& getPath() const { return spline.points;}
    };
}

