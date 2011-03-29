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

    /// Initializer for MoveSpline class
    class MoveSplineInit
    {
    public:

        explicit MoveSplineInit(UnitMovement& m);
        
        // launches initialized movement
        void Launch();

        // Adds movement by parabolic trajectory
        // max_height - the maximum height of parabola, value could be negative and positive
        // start_time - delay between movement starting time and beginning to move by parabolic trajectory
        // you can have only one parabolic motion: previous will be overriden
        // can't be combined with final animation
        MoveSplineInit& SetKnockBack(float max_height, float start_time);
        MoveSplineInit& SetTrajectory(float max_height, float start_time);
        // Plays animation after start_time delay passed (delay since movement starting time)
        // can't be combined with parabolic movement
        MoveSplineInit& SetAnimation(AnimType anim, float start_time);

        // Adds final facing animation
        // sets unit's facing to specified point/angle/target after all path done
        // you can have only one final facing: previous will be overriden
        MoveSplineInit& SetFacing(MovementBase& target);
        MoveSplineInit& SetFacing(float angle);
        MoveSplineInit& SetFacing(Vector3 const& point);

        //
        // controls - array of points, shouldn't be empty
        MoveSplineInit& MovebyPath(const PointsArray& controls, uint32 path_offset = 0);
        // Initializes spline for simple A to B motion, A is current unit's position, B is destination
        MoveSplineInit& MoveTo(const Vector3& destination);

        // Enables CatmullRom spline interpolation mode(makes path smooth)
        // if not enabled linear spline mode will be choosen
        MoveSplineInit& SetSmooth();
        // Enables CatmullRom spline interpolation mode, enables flying animation
        MoveSplineInit& SetFly();
        // Enables walk mode
        MoveSplineInit& SetWalk();
        // Makes movement cyclic
        MoveSplineInit& SetCyclic();
        // Enables falling mode
        MoveSplineInit& SetFall();

        // Sets the velocity(in case you want to have custom movement velocity)
        // if no set, speed will be selected based on values from speed table and current movement mode
        // value shouldn't be negative
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

    namespace MoveKnockBackStrategy
    {
        inline void Apply(UnitMovement& st, const Vector3& dest, float velocity)
        {
            MoveSplineInit(st).MoveTo(dest).SetKnockBack(0.5,0).SetVelocity(velocity).Launch();
        }

        inline void Apply(UnitMovement& st, const Vector3& dest, float velocity, float parabolic_heigth)
        {
            MoveSplineInit(st).MoveTo(dest).SetKnockBack(parabolic_heigth,0).SetVelocity(velocity).Launch();
        }
    };
}
