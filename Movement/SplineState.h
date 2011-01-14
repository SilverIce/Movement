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

        T operator ++()
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

    // TODO: make it Atomic
    extern counter<uint32> MoveSplineCounter;

    class MoveSpline
    {
        friend class PacketBuilder;
        friend class MoveSplineInit;
    public:

        Spline          spline;
        Vector3         finalDestination;

        union
        {
            struct Point{
                float x,y,z;
            }       facing_spot;
            uint64  facing_target;
            float   facing_angle;
        };

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

    protected:

        void partial_initialize(const PointsArray& path, float velocity, float max_parabolic_heigth);

    public:

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

