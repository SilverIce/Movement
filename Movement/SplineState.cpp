
#include "SplineState.h"


namespace Movement{

void ParabolicHandler::handleParabolic( uint32 t_duration, uint32 t_passed, Vector3& position ) const
{
    if (t_passed <= time_shift)
        return;

    float t_passedf = (t_passed - time_shift) / 1000.f;
    float t_durationf = (t_duration - time_shift) / 1000.f;

    // -a*x*x + bx + c:
    //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
    position.z += (t_durationf - t_passedf) * 0.5f * z_acceleration * t_passedf;
}

void MoveSpline::updateState( uint32 ms_time )
{
    if (splineflags & SPLINEFLAG_UNKNOWN4)
    {
        Finalize();
        return;
    }

    time_passed = getMSTimeDiff(start_move_time, ms_time);
    uint32 duration_ = float(duration) * duration_mod + 0.5f;

    if (duration_ == 0)
        return;

    if (time_passed >= duration_)
    {
        if (isCyclic())
        {
            time_passed = time_passed % duration_;
            // FIXME: will overflow if time_passed > ms_time
            // not sure that its possible
            start_move_time = ms_time - time_passed;

            if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            {
                G3D::Array<Vector3> points(spline.getPoints());
                spline.init_cyclic_spline(&points[1],points.size()-1,spline.mode(),0);

                RemoveSplineFlag(SPLINEFLAG_ENTER_CYCLE);

                // client-side bug: client resets parabolic info to default values, but doesn't removes parabolic/trajectory flag
                // in simple words: parabolic movement can be used with cyclic movement but there will be little visual bug on client side
                // i decided remove remove trajectory flag
                RemoveSplineFlag(SPLINEFLAG_TRAJECTORY);
            }

            //spline.reset_progress();

            duration_mod = this->sync_coeff;
            sync_coeff = 1.f;
        }
        else
            Finalize();
    }
}

Vector4 MoveSpline::ComputePosition() const
{
    Vector4 c;

    uint32 duration_ = float(duration) * duration_mod + 0.5f;

    float t = 0.f;
    if (Finalized())
    {
        t = 1.f;
    }
    else if (duration_ != 0)
    {
        t = float(time_passed) / float(duration_);
    }

    Vector3 hermite;
    spline.evaluate_percent_and_hermite(t, (Vector3&)c, hermite);

    if (splineflags & SPLINEFLAG_TRAJECTORY)
    {
        parabolic.handleParabolic(duration_, time_passed, (Vector3&)c);
    }
    else if (splineflags & SPLINEFLAG_FALLING)
    {
        float z_now = getNode(spline.first()).z - computeFallElevation(time_passed / 1000.f, false, 0.f);

        if (z_now < finalDestination.z)
        {
            c.z = finalDestination.z;
            log_write("MoveSpline::ComputePosition: z_now < finalDestination.z");
        }
        else
            c.z = z_now;
    }

    if (Finalized() && (splineflags & SPLINE_MASK_FINAL_FACING))
    {
        if (splineflags & SPLINEFLAG_FINAL_ANGLE)
        {
            c.w = facing_angle;
        }
        else if (splineflags & SPLINEFLAG_FINAL_POINT)
        {
            c.w = G3D::wrap(atan2(facing_spot.y-c.y, facing_spot.x-c.x), 0.f, (float)G3D::twoPi());
        }
        //nothing to do for SPLINEFLAG_FINAL_TARGET flag
    }
    else
    {
        c.w = G3D::wrap(atan2(hermite.y, hermite.x), 0.f, (float)G3D::twoPi());
    }
    return c;
}

void MoveSpline::init_spline( uint32 StartMoveTime, PointsArray& path, float velocity )
{
    start_move_time = StartMoveTime;

    duration_mod = 1.f;
    sync_coeff   = 1.f;
    time_passed  = 0;

    if (isCyclic())
    {
        uint32 cyclic_point = 0;
        if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            cyclic_point = 1;   // shouldn't be modified, came from client
        
        if (isSmooth())
            spline.init_cyclic_spline(&path[0], path.size(), SplineModeCatmullrom, cyclic_point);
        else
            spline.init_cyclic_spline(&path[0], path.size(), SplineModeLinear, cyclic_point);

        finalDestination = Vector3::zero();

        duration = spline.length(spline.first()+cyclic_point,spline.last()) / velocity * 1000.f;
    }
    else
    {
        if (isSmooth())
            spline.init_spline(&path[0], path.size(), SplineModeCatmullrom);
        else
            spline.init_spline(&path[0], path.size(), SplineModeLinear);

        finalDestination = getPath().last();

        if (splineflags & SPLINEFLAG_FALLING)
            duration = computeFallTime(path[0].z - finalDestination.z, false);
        else
            duration = spline.length() / velocity * 1000.f;
    }
    
    // TODO: where this should be handled?
    if (duration == 0)
        duration = 1;
}

void MoveSpline::init_cyclic_spline( uint32 StartMoveTime, PointsArray& path, float speed, uint32 flags )
{
    start_move_time = StartMoveTime;
    splineflags = flags | (SPLINEFLAG_CYCLIC | SPLINEFLAG_ENTER_CYCLE);

    duration_mod = 1.f;
    sync_coeff   = 1.f;

    uint32 cyclic_point = 1u;   // shouldn't be modified, came from client
    if (isSmooth())
        spline.init_cyclic_path(&path[0], path.size(), SplineModeCatmullrom, cyclic_point);
    else
        spline.init_cyclic_path(&path[0], path.size(), SplineModeLinear, cyclic_point);

    finalDestination = Vector3::zero();

    time_passed = 0;
    duration = spline.length(spline.first()+cyclic_point,spline.last()) / speed * 1000.f;

    // TODO: where this should be handled?
    if (duration == 0)
    {
        movLog.write("Error: null spline duration");
        duration = 1;
    }
}


}
