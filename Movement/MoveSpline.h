
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
        enum UpdateResult{
            Result_None         = 0x01,
            Result_Arrived      = 0x02,
            Result_NextCycle    = 0x04,
            Result_NextSegment  = 0x08,
        };
        #pragma region fields
        friend class PacketBuilder;
    protected:
        MySpline        spline;

        FacingInfo      facing;

        uint32          m_Id;

        MoveSplineFlag  splineflags;

        int32           time_passed;
        float           initialOrientation;
        float           vertical_acceleration;
        int32           effect_start_time;
        int32           point_Idx;
        int32           point_Idx_offset;

        void init_spline(const MoveSplineInitArgs& args);
    protected:

        const MySpline::ControlArray& getPath() const { return spline.getPoints();}
        void computeParabolicElevation(float& el) const;
        void computeFallElevation(float& el) const;

        UpdateResult _updateState(int32& ms_time_diff);
        int32 segment_time_elapsed() const { return next_timestamp()-time_passed;}

        void Finalize();

        #pragma endregion
    public:

        explicit MoveSpline();
        void Initialize(const MoveSplineInitArgs&);

        static bool Initialize(MoveSpline *& obj, const MoveSplineInitArgs& args)
        {
            if (!args.Validate())
                return false;
            if (obj == NULL)
                obj = new MoveSpline();
            obj->Initialize(args);
            return true;
        }

        template<class UpdateHandler>
        void updateState(int32 difftime, UpdateHandler& handler)
        {
            do
                handler(_updateState(difftime));
            while(difftime > 0);
        }

        void updateState(int32 difftime)
        {
            do _updateState(difftime);
            while(difftime > 0);
        }

        Location ComputePosition() const;

        uint32 GetId() const { return m_Id;}
        bool Arrived() const { return Duration() == timePassed(); }
        bool isCyclic() const { return splineflags.cyclic;}
        int32 Duration() const { return spline.length();}
        int32 timeElapsed() const { return Duration() - time_passed;}
        int32 timePassed() const { return time_passed;}
        int32 next_timestamp() const { return spline.length(point_Idx+1);}
        const Vector3& FinalDestination() const { return spline.getPoint(spline.last());}
        const Vector3& CurrentDestination() const { return spline.getPoint(point_Idx+1);}
        int32 currentPathIdx() const;

        std::string ToString() const;
    };
}

