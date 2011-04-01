#include "UnitMovement.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Object.h"
#include "MoveUpdater.h"

namespace Movement
{
    MoveSplineInit& MoveSplineInit::MovebyPath(const PointsArray& controls, uint32 path_offset, bool contains_current)
    {
        if (!contains_current)
        {
            args.path_Idx_offset = path_offset - 1;
            args.path.resize(controls.size()+1);
            std::copy(controls.begin(),controls.end(),args.path.begin()+1);
        } 
        else
        {
            args.path_Idx_offset = 0;
            args.path.assign(controls.begin(),controls.end());
        }
        return *this;
    }

    MoveSplineInit& MoveSplineInit::MoveTo(const Vector3& dest)
    {
        args.path_Idx_offset = 0;
        args.path.resize(2);
        args.path[1] = dest;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFly()
    {
        args.flags.EnableFlying();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetWalk(bool enable)
    {
        args.flags.walkmode = enable;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetSmooth()
    {
        args.flags.EnableCatmullRom();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetCyclic()
    {
        args.flags.cyclic = true;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFall()
    {
        args.flags.EnableFalling();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetVelocity(float vel)
    {
        args.velocity = vel;
        return *this;
    }

    void MoveSplineInit::Launch()
    {
        //correct first vertex
        args.path[0] = state.GetPosition3();

        // select velocity
        if (args.velocity != 0.f)
        {
            state.speed_obj.current = args.velocity;
            state.speed_type = SpeedNotStandart;
        }
        else
        {
            state.ReCalculateCurrentSpeed();
            args.velocity = state.GetCurrentSpeed();
        }

        args.splineId = state.GetUpdater().NewMoveSplineId();

        if (target)
            state.BindOrientationTo(*target);
        else
            state.UnbindOrientation();

        // no sense to move unit
        // TODO: find more elegant way (maybe just set current_speed to some minimal value)
        if (args.velocity > 0.f)
        {
            MoveSplineSegmented& spline = state.move_spline;
            spline.Initialize(args);

            state.EnableSpline();
            state.SetForwardDirection();

            state.ScheduleUpdate();
            // shall MoveSpline initializer care about packet broadcasting?
            PacketBuilder::SplinePathSend(state, MsgBroadcast(state));
        }
    }

    MoveSplineInit& MoveSplineInit::SetParabolic(float max_height, float time_shift, bool is_knockback)
    {
        args.time_perc = time_shift;
        args.parabolic_heigth = max_height;
        args.flags.EnableParabolic();
        args.flags.knockback = is_knockback;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing(MovementBase& t)
    {
        args.facing.target = t.Owner.GetGUID();
        args.flags.EnableFacingTarget();
        target = &t;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing(float o)
    {
        args.facing.angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing(Vector3 const& spot)
    {
        args.facing.x = spot.x;
        args.facing.y = spot.y;
        args.facing.z = spot.z;
        args.flags.EnableFacingPoint();
        return *this;
    }

    MoveSplineInit::MoveSplineInit(UnitMovement& m) : state(m), target(NULL)
    {
        state.UpdateState();
    }

    MoveSplineInit& MoveSplineInit::SetAnimation(AnimType anim, float anim_time)
    {
        args.time_perc = anim_time;
        args.flags.EnableAnimation(anim);
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetBackward()
    {
        args.flags.backward = true;
        return *this;
    }
}
