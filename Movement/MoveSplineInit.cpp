#include "MoveSplineInit.h"
#include "UnitMovement.h"

namespace Movement
{
    MoveSplineInit& MoveSplineInit::MovebyPath( PointsArray& controls, uint32 path_offset )
    {
        args.path_Idx_offset = path_offset - 1;
        args.path.swap(controls);
        return *this;
    }

    MoveSplineInit& MoveSplineInit::MoveTo( const Vector3& dest )
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

    MoveSplineInit& MoveSplineInit::SetWalk()
    {
        args.flags.walkmode = true;
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

    MoveSplineInit& MoveSplineInit::SetVelocity( float vel )
    {
        args.velocity = vel;
        return *this;
    }

    void MoveSplineInit::Launch()
    {
        if (target)
            state.BindOrientationTo(*target);
        else
            state.UnbindOrientation();

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

        // no sense to move unit
        // TODO: find more elegant way (maybe just set current_speed to some minimal value)
        if (args.velocity > 0.f)
        {
            args.path[0] = state.GetPosition3();

            MoveSplineUsed& spline = state.move_spline;
            spline.Initialize(args);

            state.EnableSpline();
            state.SetForwardDirection();

            state.SheduleUpdate(spline.Duration());
            // shall MoveSpline initializer care about packet broadcasting?
            PacketBuilder::PathUpdate(state, MsgBroadcast(state));
        }
    }

    MoveSplineInit& MoveSplineInit::SetTrajectory( float max_height, float time_shift )
    {
        args.time_perc = time_shift;
        args.parabolic_heigth = max_height;
        args.flags.EnableParabolic();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetKnockBack( float max_height, float time_shift )
    {
        SetTrajectory(max_height, time_shift);
        args.flags.knockback = true;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing( MovementBase& t )
    {
        args.facing.target = t.GetOwner().GetGUID();
        args.flags.EnableFacingTarget();
        target = &t;
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing( float o )
    {
        args.facing.angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
        return *this;
    }

    MoveSplineInit& MoveSplineInit::SetFacing( Vector3 const& spot )
    {
        args.facing.spot.x = spot.x;
        args.facing.spot.y = spot.y;
        args.facing.spot.z = spot.z;
        args.flags.EnableFacingPoint();
        return *this;
    }

    MoveSplineInit::MoveSplineInit(MovementState& m) : state(m), target(NULL)
    {
        state.UpdateState();
    }

    MoveSplineInit& MoveSplineInit::SetAnimation(AnimType anim, float anim_time)
    {
        args.time_perc = anim_time;
        args.flags.EnableAnimation(anim);
        return *this;
    }
}
