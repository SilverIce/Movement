/**
  file base:    MoveSplineInit.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      19:2:2011
*/

#pragma once

#include "framework/typedefs.h"

namespace Movement
{
    class MovementBase;
    class UnitMovement;
    class UnitMovementImpl;

    typedef std::vector<Vector3> PointsArray;

    enum AnimType
    {
        UNK0 = 0, // 460 = ToGround, index of AnimationData.dbc
        UNK1 = 1, // 461 = FlyToFly?
        UNK2 = 2, // 458 = ToFly
        UNK3 = 3, // 463 = FlyToGround
    };

    /*  Initializes and launches spline movement
     */
    class EXPORT MoveSplineInit
    {
    public:

        explicit MoveSplineInit(UnitMovement& m);
        explicit MoveSplineInit(UnitMovementImpl& m);
        ~MoveSplineInit();

        /*  Final pass of initialization that launches spline movement.
         */
        void Launch();

        /* Adds movement by parabolic trajectory
         * @param amplitude  - the maximum height of parabola, value could be negative and positive
         * @param start_time - delay between movement starting time and beginning to move by parabolic trajectory
         * can't be combined with final animation
         */
        void SetParabolic(float amplitude, float start_time);
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
        void SetFirstPointId(int32 pointId);

        /* Enables CatmullRom spline interpolation mode(makes path smooth)
         * if not enabled linear spline mode will be choosen
         */
        void SetSmooth();
        /* Enables CatmullRom spline interpolation mode, enables flying animation
         */
        void SetFly();
        /* Enables walk mode
         */
        void SetWalk(bool enable);
        /* Makes movement cyclic
         */
        void SetCyclic();
        /* Enables falling mode
         */
        void SetFall();
        /* Inverses unit model orientation. Disabled by default
         */
        void SetOrientationInversed();
        /* Prevents unit model from being oriented. Disabled by default
         */
        void SetOrientationFixed(bool enable);
        /* Instantly moves unit to end of the path. Disabled by default
         */
        void SetInstant();

        /* Sets the velocity (in case you want to have custom movement velocity)
         * if no set, speed will be selected based on unit's speeds and current movement mode
         * Has no effect if falling mode enabled
         * velocity shouldn't be negative
         */
        void SetVelocity(float velocity);

        PointsArray& Path();

    private:

        struct MoveSplineInitArgs& args;
        UnitMovementImpl&  state;
        char args_Size[76];
    };

    inline void MoveJumpInit(UnitMovementImpl& st, const Vector3& dest, float velocity, float parabolic_heigth = 0.5f)
    {
        MoveSplineInit init(st);
        init.MoveTo(dest);
        init.SetParabolic(parabolic_heigth,0);
        init.SetVelocity(velocity);
        init.Launch();
    }

    struct OnEventArgs
    {
        enum EventType{
            PointDone,
            Arrived,
        };

        static OnEventArgs OnArrived(uint32 splineId)
        {
            OnEventArgs args = {Arrived, splineId, 0};
            return args;
        }

        static OnEventArgs OnPoint(uint32 splineId, int32 pointId)
        {
            OnEventArgs args = {PointDone, splineId, pointId};
            return args;
        }

        bool isArrived() const { return type == Arrived;}
        bool isPointDone() const { return type == PointDone;}

        EventType type;
        uint32 splineId;
        int32 data;
    };
}
