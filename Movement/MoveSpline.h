
/**
  file:         MoveSpline.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "spline.h"
#include "MoveSplineInitArgs.h"
#include "Location.h"

namespace Movement {


    // MoveSpline - кривая гладкая или ломаная линия и точка на ней, движущаяся из начала в конец кривой
    // точка может иметь вертикальную составляющую движения
    // кривая может быть замкнута - в этом случае точка никогда не остановится
    // состояние MoveSpline необратимо: точка может двигаться только вперед

    // MoveSpline represents smooth catmullrom or linear curve and point that moves belong it
    // curve can be cyclic - in this case movement will be cyclic
    // point can have vertical acceleration motion componemt(used in fall, parabolic movement)
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

        MoveSplineFlag  splineflags;

        int32           time_passed;
        // currently duration mods are unused, but its _currently_
        //float           duration_mod;
        //float           duration_mod_next;
        float           vertical_acceleration;
        int32           spec_effect_time;

        void init_spline(const MoveSplineInitArgs& args);
    protected:
        bool isCyclic() const { return splineflags.cyclic;}
        bool isSmooth() const { return splineflags.isSmooth();}
        void Finalize() { splineflags.done = true; }
        void RemoveSplineFlag(uint32 f) { splineflags &= ~f;}
        uint32 GetSplineFlags() const { return splineflags.raw;}

        const PointsArray& getPath() const { return spline.getPoints();}
        void computeParabolicElevation(float& el) const;
        void computeFallElevation(float& el) const;

        void OnArrived();

        Location _ComputePosition(MySpline::index_type Idx, float u) const;

        #pragma endregion
    public:

        void Initialize(const MoveSplineInitArgs&);
        bool Initialized() const { return !spline.empty();}

        explicit MoveSpline();

        enum UpdateResult{
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08,
            Result_StopUpdate   = 0x10,
        };

        UpdateResult updateState( int32 ms_time_diff );
        Location ComputePosition() const;

        uint32 GetId() const { return m_Id;}
        bool Finalized() const { return splineflags.done; }
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
        #pragma endregion
    public:

        explicit MoveSplineSegmented();

        /** Just a little example of 'How to update':
         ** int32 ms_time_diff = 5000;
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
        Location ComputePosition() const;

        void Initialize(const MoveSplineInitArgs&);

        int32 currentPathSegment() const { return point_Idx_offset + point_Idx - spline.first();}
        int32 currentSplineSegment() const { return point_Idx;}

        std::string ToString() const;
    };
}

