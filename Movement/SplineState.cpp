
#include "SplineState.h"
#include <sstream>


namespace Movement{

// TODO: make it Atomic
counter<uint32> MoveSplineCounter;



void MoveSpline::updateState( int32 ms_time_diff )
{
    if (splineflags & SPLINEFLAG_UNKNOWN4)
    {
        Finalize();
        return;
    }

    //time_passed = getMSTimeDiff(start_move_time, ms_time);
    time_passed += ms_time_diff;
    uint32 duration_ = modifiedDuration();

    if (duration_ == 0)
        return;

    if (time_passed >= duration_)
    {
        if (isCyclic())
        {
            time_passed = time_passed % duration_;
            // FIXME: will overflow if time_passed > ms_time
            // not sure that its possible
            //start_move_time = ms_time - time_passed;

            if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            {
                PointsArray points(spline.getPoints());
                spline.init_cyclic_spline(&points[spline.first()+1],spline.pointsCount()-1,spline.mode(),0);

                RemoveSplineFlag(SPLINEFLAG_ENTER_CYCLE);

                // client-side bug: client resets parabolic info to default values, but doesn't removes parabolic/trajectory flag
                // in simple words: parabolic movement can be used with cyclic movement but there will be little visual bug on client side
                // i decided remove remove trajectory flag
                RemoveSplineFlag(SPLINEFLAG_TRAJECTORY);
            }

            duration_mod = this->duration_mod_next;
            duration_mod_next = 1.f;

            duration_ = modifiedDuration();
        }
        else
            Finalize();
    }
}

Vector4 MoveSpline::ComputePosition() const
{
    uint32 duration_ = modifiedDuration();

    float t = 0.f;
    if (Finalized())
    {
        t = 1.f;
    }
    else if (duration_ != 0)
    {
        t = float(time_passed) / float(duration_);
    }

    Vector4 c; 
    Vector3 hermite;
    spline.evaluate_percent_and_hermite(t, (Vector3&)c, hermite);

    if (splineflags & SPLINEFLAG_TRAJECTORY)
    {
        if (time_passed > parabolic_time)
        {
            float t_passedf = (time_passed - parabolic_time) / 1000.f;
            float t_durationf = (duration - parabolic_time) / 1000.f; //client use not modified duration here

            // -a*x*x + bx + c:
            //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
            c.z += (t_durationf - t_passedf) * 0.5f * parabolic_acceleration * t_passedf;
        }
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

void MoveSpline::partial_initialize(const PointsArray& path, float velocity, float max_parabolic_heigth)
{
    static Spline::EvaluationMode modes[2] = {Spline::ModeLinear,Spline::ModeCatmullrom};

    if (isCyclic())
    {
        uint32 cyclic_point = 0;
        if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            cyclic_point = 1;   // shouldn't be modified, came from client
        
        spline.init_cyclic_spline(&path[0], path.size(), modes[isSmooth()], cyclic_point);

        finalDestination = Vector3::zero();

        duration = spline.length(spline.first()+cyclic_point,spline.last()) / velocity * 1000.f;
    }
    else
    {
        spline.init_spline(&path[0], path.size(),  modes[isSmooth()]);

        finalDestination = spline.getPoint(spline.last());

        if (splineflags & SPLINEFLAG_FALLING)
            duration = computeFallTime(path[0].z - finalDestination.z, false);
        else
            duration = spline.length() / velocity * 1000.f;
    }
    
    // TODO: where this should be handled?
    if (duration == 0)
        duration = 1;

    // path initialized, duration is known and i able to compute z_acceleration for parabolic movement
    if (splineflags & SPLINEFLAG_TRAJECTORY)
    {
        float f_duration = (duration - parabolic_time) / 1000.f;
        parabolic_acceleration = max_parabolic_heigth * 8.f / (f_duration * f_duration);
    }
}

std::string MoveSpline::ToString() const
{
    std::stringstream str;

    str << "MoveSpline" << std::endl;
    str << "spline Id:    " << GetId() << std::endl;
    str << "spline flags: " << GetSplineFlags() << std::endl;
    str << "duration:     " << duration << std::endl;
    str << "time passed:  " << time_passed << std::endl;
    str << spline.ToString();
    return str.str();
}

MoveSpline::MoveSpline() : m_Id(0), splineflags(0),
    time_passed(0), duration(0), duration_mod(1.f), duration_mod_next(1.f),
    parabolic_acceleration(1.f), parabolic_time(0)
{
}
}
