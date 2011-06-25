/**
  file base:    MoveSplineInit.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      19:2:2011
*/

#pragma once

#include "mov_constants.h"
#include "MoveSplineInitArgs.h"

namespace Movement
{
    class MovementBase;
    class UnitMovement;

    /*  Initializes and launches spline movement
     */
    class MoveSplineInit
    {
    public:

        explicit MoveSplineInit(UnitMovement& m);
        
        /*  Final pass of initialization that launches spline movement.
         */
        void Launch();

        /* Adds movement by parabolic trajectory
         * @param amplitude  - the maximum height of parabola, value could be negative and positive
         * @param start_time - delay between movement starting time and beginning to move by parabolic trajectory
         * can't be combined with final animation
         */ 
        void SetParabolic(float amplitude, float start_time, bool is_knockback = false);
        /* Plays animation after start_time delay passed (delay since movement starting time)
         * can't be combined with parabolic movement
         */
        void SetAnimation(AnimType anim, float start_time);

        /* Adds final facing animation
         * sets unit's facing to specified point/angle after all path done
         * you can have only one final facing: previous will be overriden
         */ 
        void SetFacing(float angle);
        void SetFacing(Vector3 const& point);

        /* Initializes movement by path
         * @param path - array of points, shouldn't be empty
         * @param pointId - Id of fisrt point of the path. Example: when third path point will be done it will notify that pointId + 3 done
         */ 
        void MovebyPath(const PointsArray& path, int32 pointId = 0);

        /* Initializes simple A to B mition, A is current unit's position, B is destination
         */ 
        void MoveTo(const Vector3& destination);

        /* Sets Id of fisrt point of the path. When N-th path point will be done ILisener will notify that pointId + N done
         * Needed for waypoint movement where path splitten into parts
         */
        void SetFirstPointId(int32 pointId) { args.path_Idx_offset = pointId; }

        /* Enables CatmullRom spline interpolation mode(makes path smooth)
         * if not enabled linear spline mode will be choosen
         */
        void SetSmooth();
        /* Enables CatmullRom spline interpolation mode, enables flying animation
         */ 
        void SetFly();
        /* Enables walk mode. Disabled by default
         */ 
        void SetWalk(bool enable);
        /* Makes movement cyclic
         */
        void SetCyclic();
        /* Enables falling mode
         */
        void SetFall();
        /* 
         */
        void SetBackward();

        /* Sets the velocity (in case you want to have custom movement velocity)
         * if no set, speed will be selected based on unit's speeds and current movement mode
         * Has no effect if falling mode enabled
         * velocity shouldn't be negative
         */ 
        void SetVelocity(float velocity);

        PointsArray& Path() { return args.path; }
        const UnitMovement& Movement() { return state; }

        template<typename InitStrategy>
        inline MoveSplineInit& operator << (InitStrategy init)
        {
            init(args);
            return *this;
        }

    protected:

        MoveSplineInitArgs args;
        UnitMovement&  state;
    };
    
    /*  I have a feeling that blizz have 5 movespline initializers:
     */

    class MoveCommonInit : private MoveSplineInit
    {
    public:

        explicit MoveCommonInit(UnitMovement& m) : MoveSplineInit(m) {}

        void Launch();

        using MoveSplineInit::MoveTo;
        using MoveSplineInit::MovebyPath;
        using MoveSplineInit::Path;
        using MoveSplineInit::SetVelocity;
        using MoveSplineInit::SetFirstPointId;

        void SetFacing(float angle) {MoveSplineInit::SetFacing(angle);}
        //void SetFacing(MovementBase& target);
        //void Add_Unk6Flag();
    };

    class MoveParabolicInit : private MoveSplineInit
    {
    public:
        MoveParabolicInit(UnitMovement& m) : MoveSplineInit(m) {}

        void MovebyTrajectory(const Vector3& dest, float amplitude, float velocity, bool knockback = false, float time_perc = 0.f);
        void MovebyTrajectory(const PointsArray& path, float amplitude, float velocity, bool knockback = false, float time_perc = 0.f);

        //void SetWalk();
        //void Add_Unk8Flag();
    };

    class MoveFallInit
    {
        MoveSplineInit init;
    public:
        MoveFallInit(UnitMovement& m, const Vector3& destination) : init(m)
        {
            init.MoveTo(destination);
            init.SetFall();
            init.Launch();
        }
    };

    class MoveCyclicInit : private MoveSplineInit
    {
    public:

        MoveCyclicInit(UnitMovement& m) : MoveSplineInit(m) {}

        void Launch();
        using MoveSplineInit::SetVelocity;
        using MoveSplineInit::Path;
        using MoveSplineInit::SetFirstPointId;
    };
/*
    class MoveAnimInit
    {
    public:
        MoveAnimInit(UnitMovement& m) : MoveSplineInit(m) {}
        void Launch();

        void SetWalk();
        void SetFly();
        void MoveTo(const Vector3& dest);
    };
*/
    inline void MoveJumpInit(UnitMovement& st, const Vector3& dest, float velocity, float parabolic_heigth = 0.5f)
    {
        MoveSplineInit init(st);
        init.MoveTo(dest);
        init.SetParabolic(parabolic_heigth,0,false);
        init.SetVelocity(velocity);
        init.Launch();
    }
}
