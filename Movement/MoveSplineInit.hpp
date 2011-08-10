#pragma once

namespace Movement
{
    void MoveSplineInit::MovebyPath(const PointsArray& controls, int32 path_offset)
    {
        args.path_Idx_offset = path_offset;
        args.path.assign(controls.begin(),controls.end());
    }

    void MoveSplineInit::MoveTo(const Vector3& dest)
    {
        args.path_Idx_offset = 0;
        args.path.resize(2);
        args.path[1] = dest;
    }

    void MoveSplineInit::SetFly()
    {
        args.flags.EnableFlying();
    }

    void MoveSplineInit::SetWalk(bool enable)
    {
        args.flags.walkmode = enable;
    }

    void MoveSplineInit::SetSmooth()
    {
        args.flags.EnableCatmullRom();
    }

    void MoveSplineInit::SetCyclic()
    {
        args.flags.cyclic = true;
    }

    void MoveSplineInit::SetFall()
    {
        args.flags.EnableFalling();
    }

    void MoveSplineInit::SetVelocity(float vel)
    {
        args.velocity = vel;
    }

    void MoveSplineInit::Launch()
    {
        if (args.path.empty())
        {
            // TODO: should i do the things that user should do?
            MoveTo(state.GetPosition3());
        }

        state.LaunchMoveSpline(args);
    }

    void MoveSplineInit::SetParabolic(float amplitude, float time_shift)
    {
        args.time_perc = time_shift;
        args.parabolic_amplitude = amplitude;
        args.flags.EnableParabolic();
    }

    void MoveSplineInit::SetFacing(float o)
    {
        args.facing.angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
    }

    void MoveSplineInit::SetFacing(Vector3 const& spot)
    {
        args.facing.x = spot.x;
        args.facing.y = spot.y;
        args.facing.z = spot.z;
        args.flags.EnableFacingPoint();
    }

    MoveSplineInit::MoveSplineInit(UnitMovement& m) : state(m.Impl())
    {
        args.flags.walkmode = m.IsWalking();
        args.flags.flying = m.IsFlying();
    }

    MoveSplineInit::MoveSplineInit(UnitMovementImpl& m) : state(m)
    {
        args.flags.walkmode = m.IsWalking();
        args.flags.flying = m.IsFlying();
    }

    void MoveSplineInit::SetAnimation(AnimType anim, float anim_time)
    {
        args.time_perc = anim_time;
        args.flags.EnableAnimation(anim);
    }

    void MoveSplineInit::SetOrientationInversed()
    {
        args.flags.backward = true;
    }

    void MoveSplineInit::SetOrientationFixed(bool enable)
    {
        args.flags.rotation_fixed = enable;
    }
}
