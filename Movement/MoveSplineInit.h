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
        MoveSplineInit& SetParabolic(float amplitude, float start_time, bool is_knockback = false);
        /* Plays animation after start_time delay passed (delay since movement starting time)
         * can't be combined with parabolic movement
         */
        MoveSplineInit& SetAnimation(AnimType anim, float start_time);

        /* Adds final facing animation
         * sets unit's facing to specified point/angle/target after all path done
         * you can have only one final facing: previous will be overriden
         */ 
        MoveSplineInit& SetFacing(MovementBase& target);
        MoveSplineInit& SetFacing(float angle);
        MoveSplineInit& SetFacing(Vector3 const& point);

        /* Initializes movement by path
         * @param path - array of points, shouldn't be empty
         * @param pointId - Id of fisrt point of the path. Example: when third path point will be done it will notify that pointId + 3 done
         * @param contains_current - set to true if first path point is current unit's position
         */ 
        MoveSplineInit& MovebyPath(const PointsArray& path, int32 pointId = 0, bool contains_current = false);
        /* Initializes simple A to B mition, A is current unit's position, B is destination
         */ 
        MoveSplineInit& MoveTo(const Vector3& destination);

        /* Enables CatmullRom spline interpolation mode(makes path smooth)
         * if not enabled linear spline mode will be choosen
         */
        MoveSplineInit& SetSmooth();
        /* Enables CatmullRom spline interpolation mode, enables flying animation
         */ 
        MoveSplineInit& SetFly();
        /* Enables walk mode
         */ 
        MoveSplineInit& SetWalk(bool enable);
        /* Makes movement cyclic
         */
        MoveSplineInit& SetCyclic();
        /* Enables falling mode
         */
        MoveSplineInit& SetFall();
        /* 
         */
        MoveSplineInit& SetBackward();

        /* Sets the velocity (in case you want to have custom movement velocity)
         * if no set, speed will be selected based on unit's speeds and current movement mode
         * Has no effect if falling mode enabled
         * velocity shouldn't be negative
         */ 
        MoveSplineInit& SetVelocity(float velocity);

        template<typename InitStrategy>
        inline MoveSplineInit& operator << (InitStrategy init)
        {
            init(args);
            return *this;
        }

    private:

        MoveSplineInitArgs args;
        UnitMovement&  state;
        MovementBase*   target;

        // lets prevent dynamic allocation: that object should have short lifetime
        void* operator new(size_t);
    };

    {

    };
    inline void MoveJumpInit(UnitMovement& st, const Vector3& dest, float velocity, float parabolic_heigth = 0.5f)
    {
        MoveSplineInit(st).MoveTo(dest).SetParabolic(parabolic_heigth,0,false).SetVelocity(velocity).Launch();
    }
}
