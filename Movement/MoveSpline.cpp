
#include "MoveSpline.h"
#include <sstream>


namespace Movement{

// TODO: make it Atomic
MoveSplineCounter movespline_counter;



void MoveSpline::updateState( uint32 ms_time_diff )
{
    mov_assert(Initialized());

    if (splineflags & SPLINEFLAG_INSTANT)
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
                // client resets duration mods
                //duration_mod = 1.f;
                //duration_mod_next = 1.f;
            }


            //duration_mod = this->duration_mod_next;
            //duration_mod_next = 1.f;
            duration_ = modifiedDuration();
        }
        else
            Finalize();
    }
}

Vector4 MoveSpline::ComputePosition() const
{
    mov_assert(Initialized());

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
        if (time_passed > spec_effect_time)
        {
            float t_passedf = MSToSec(time_passed - spec_effect_time);
            float t_durationf = MSToSec(duration - spec_effect_time); //client use not modified duration here

            // -a*x*x + bx + c:
            //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
            c.z += (t_durationf - t_passedf) * 0.5f * vertical_acceleration * t_passedf;
        }
    }
    else if (splineflags & SPLINEFLAG_FALLING)
    {
        float z_now = spline.getPoint(spline.first()).z - computeFallElevation(MSToSec(time_passed), false, 0.f);

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
            c.w = facing.angle;
        }
        else if (splineflags & SPLINEFLAG_FINAL_POINT)
        {
            c.w = G3D::wrap(atan2(facing.spot.y-c.y, facing.spot.x-c.x), 0.f, (float)G3D::twoPi());
        }
        //nothing to do for SPLINEFLAG_FINAL_TARGET flag
    }
    else
    {
        c.w = G3D::wrap(atan2(hermite.y, hermite.x), 0.f, (float)G3D::twoPi());
    }
    return c;
}

inline uint32 computeDuration(float length, float velocity)
{
    return SecToMS(length / velocity);
}

void MoveSpline::Initialize(const MoveSplineInitArgs& args)
{
    mov_assert(args.path.size() > 1);
    mov_assert(args.velocity > 0.f);
    mov_assert(args.time_perc >= 0.f && args.time_perc <= 1.f);

    splineflags = args.flags;
    facing = args.facing;

    time_passed  = 0;
    //duration_mod = 1.f;
    //duration_mod_next = 1.f;
    vertical_acceleration = 0.f;
    spec_effect_time = 0;

    /*  checks spline flags, removes not compartible
    if (splineflags & SPLINEFLAG_CYCLIC && !(isSmooth()))
        splineflags &= ~SPLINEFLAG_CYCLIC;

    if (splineflags & SPLINEFLAG_ANIMATION)
        splineflags &= ~(SPLINEFLAG_TRAJECTORY|SPLINEFLAG_FALLING|SPLINEFLAG_KNOCKBACK);
    else if (splineflags & SPLINEFLAG_TRAJECTORY)
        splineflags &= ~SPLINEFLAG_FALLING;
    */

    static Spline::EvaluationMode modes[2] = {Spline::ModeLinear,Spline::ModeCatmullrom};

    if (isCyclic())
    {
        uint32 cyclic_point = 0;
        if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            cyclic_point = 1;   // shouldn't be modified, came from client

        spline.init_cyclic_spline(&args.path[0], args.path.size(), modes[isSmooth()], cyclic_point);
        finalDestination = Vector3::zero();

        duration = computeDuration(spline.length(spline.first()+cyclic_point,spline.last()),args.velocity);
    }
    else
    {
        spline.init_spline(&args.path[0], args.path.size(), modes[isSmooth()]);
        finalDestination = spline.getPoint(spline.last());

        if (splineflags & SPLINEFLAG_FALLING)
            duration = computeFallTime(args.path[0].z - finalDestination.z, false);
        else
            duration = computeDuration(spline.length(),args.velocity);
    }

    if (!duration)
        duration = 1;

    segment_Idx = spline.first();

    // path initialized, duration is known and i able to compute parabolic acceleration
    if (splineflags & (SPLINEFLAG_TRAJECTORY|SPLINEFLAG_ANIMATION))
    {
        spec_effect_time = duration * args.time_perc;
        if (splineflags & SPLINEFLAG_TRAJECTORY)
        {
            float f_duration = MSToSec(duration - spec_effect_time);
            vertical_acceleration = args.parabolic_heigth * 8.f / (f_duration * f_duration);
        }
    }

    m_Id = movespline_counter.NewId();
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

MoveSpline::MoveSpline() : m_Id(MoveSplineCounter::Lower_limit), splineflags(0),
    time_passed(0), duration(0), //duration_mod(1.f), duration_mod_next(1.f),
    vertical_acceleration(1.f), spec_effect_time(0)
{
}

}
