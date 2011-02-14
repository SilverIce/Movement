#pragma once

#include "mov_constants.h"
#include "spline.h"
#include "G3D/Vector4.h"

#include <limits>

namespace Movement {

    template<class T, T lower_limit>
    class counter
    {
    public:
        counter() { init();}

        T NewId()
        {
            if (m_counter == std::numeric_limits<T>::max())
                init();
            else
                ++m_counter;
            return m_counter;
        }

        enum{
            Lower_limit = lower_limit,
        };
    private:
        void init() { m_counter = lower_limit+1; }
        T m_counter;
    };

    // TODO: make it Atomic
    typedef counter<uint32, 0> MoveSplineCounter;
    extern MoveSplineCounter movespline_counter;

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
        MoveSplineInitArgs() : flags(0), path_Idx_offset(0),
            velocity(0.f), parabolic_heigth(0.f), time_perc(0.f)   {}

        PointsArray path;
        FacingInfo facing;
        uint32 flags;
        int32 path_Idx_offset;
        float velocity;
        float parabolic_heigth;
        float time_perc;

        bool Validate() const;
    private:
        bool _checkPathBounds() const;
    };

    class MoveSpline
    {
    public:
        typedef Spline<int32> MySpline;
        #pragma region fields
        friend class PacketBuilder;
    protected:
        MySpline        spline;
        Vector3         finalDestination;

        FacingInfo      facing;

        uint32          m_Id;

        union{
            uint8       animation_type;
            uint32      splineflags;
        };

        int32          time_passed;
        // currently duration mods are unused, but its _currently_
        //float           duration_mod;
        //float           duration_mod_next;
        float           vertical_acceleration;
        uint32          spec_effect_time;

    protected:
        bool isCyclic() const { return splineflags & SPLINEFLAG_CYCLIC;}
        bool isSmooth() const { return splineflags & (SPLINEFLAG_FLYING|SPLINEFLAG_CATMULLROM);}
        void Finalize() { splineflags |= SPLINEFLAG_DONE; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f;}
        uint32 GetSplineFlags() const { return splineflags;}

        const PointsArray& getPath() const { return spline.getPoints();}
        void computeParabolicElevation(float& el) const;
        void computeFallElevation(float& el) const;

        void OnArrived();

        Vector4 _ComputePosition(MySpline::index_type Idx, float u) const;

        #pragma endregion
    public:

        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return GetId()!= MoveSplineCounter::Lower_limit;}

        explicit MoveSpline();

        enum UpdateResult{
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08,
            Result_StopUpdate   = 0x10,
        };

        UpdateResult updateState( int32 ms_time_diff );
        Vector4 ComputePosition() const;

        uint32 GetId() const { return m_Id;}
        bool Finalized() const { return splineflags & SPLINEFLAG_DONE; }
        int32 Duration() const { return spline.length();}
        int32 timeElapsed() const { return Duration() - time_passed;}
        int32 timePassed() const { return time_passed;}
        const Vector3& FinalDestination() const { return finalDestination;}

        // not supported functions:
        int32 currentPathSegment() const;
        int32 currentSplineSegment() const;
        //UpdateResult updateState(int32& ms_time_diff);

        std::string ToString() const;

    };

    class MoveSplineSegmented : public MoveSpline
    {
        #pragma region fields
    protected:
        int32           point_Idx;
        int32           point_Idx_offset;

    protected:
        UpdateResult _updateState( int32 ms_time_diff );
        int32 segment_timestamp() const { return spline.length(point_Idx+1);}
        void init_segment(int32 seg);

        #pragma endregion
    public:

        explicit MoveSplineSegmented();

        /** Just a little example of 'How to update':
         ** int32 difftime = 5000;
         ** UpdateResult result;
         ** do {
         **     UpdateResult result = move_spline.updateState(ms_time_diff);
         **     switch(result & ~Result_StopUpdate)
         **     {
         **     case Result_NextCycle:
         **     ...
         **     case Result_Arrived:
         **     ...
         **     }
         ** } while(!(result & Result_StopUpdate))
        */
        UpdateResult updateState(int32& ms_time_diff);
        Vector4 ComputePosition() const;

        void Initialize(const MoveSplineInitArgs&);

        int32 currentPathSegment() const { return point_Idx_offset + point_Idx - spline.first();}
        int32 currentSplineSegment() const { return point_Idx;}
    };

    typedef MoveSplineSegmented MoveSplineUsed;
}

