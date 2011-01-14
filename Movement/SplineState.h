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
    };

    class MoveSpline
    {
        friend class PacketBuilder;
    private:
        #pragma region fields
        Spline          spline;
        Vector3         finalDestination;

        FacingInfo      facing;

        uint32          m_Id;

        union{
            uint8       animation_type;
            uint32      splineflags;
        };

        //uint32          start_move_time;
        uint32          time_passed;
        uint32          duration;
        float           duration_mod;
        float           duration_mod_next;
        float           parabolic_acceleration;
        union{
            uint32      parabolic_time;
            uint32      animation_time;
        };
        #pragma endregion
    public:

        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return GetId()!= MoveSplineCounter::Lower_limit;}

        explicit MoveSpline();

        void updateState( int32 ms_time_diff );
        Vector4 ComputePosition() const;

        // helpers
        bool Finalized() const { return splineflags & SPLINEFLAG_DONE; }
        bool isCyclic() const { return splineflags & SPLINEFLAG_CYCLIC;}
        bool isSmooth() const { return splineflags & (SPLINEFLAG_FLYING|SPLINEFLAG_CATMULLROM);}
        uint32 GetSplineFlags() const { return splineflags;}
        uint32 GetId() const { return m_Id;}

        int32 modifiedDuration() const { return duration_mod * duration + 0.5f;}
        int32 timeElapsed() const { return modifiedDuration() - time_passed;}
        const Vector3& getNode(uint32 i) const { return spline.getPoints()[i];}
        const PointsArray& getPath() const { return spline.getPoints();}

        std::string ToString() const;

    private:
        void Finalize() { splineflags |= SPLINEFLAG_DONE; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f;}
    };
}

