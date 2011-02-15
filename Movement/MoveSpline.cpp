
#include "MoveSpline.h"
#include <sstream>

namespace Movement{

// TODO: make it Atomic
MoveSplineCounter movespline_counter;

MoveSpline::UpdateResult MoveSpline::updateState( int32 ms_time_diff )
{
    mov_assert(Initialized());
    mov_assert(ms_time_diff >= 0);

    UpdateResult result = Result_None;
    if (!Finalized())
    {
        //if (splineflags & SPLINEFLAG_INSTANT)
        //    Finalize();
        time_passed += ms_time_diff;
        if (time_passed >= Duration())
        {
            OnArrived();
            result = Finalized() ? Result_Arrived : Result_NextCycle;
        }
    }
    return result;
}

Vector4 MoveSpline::ComputePosition() const
{
    SplineBase::index_type seg_Idx;
    float u, t = 1.f;
    if (Duration())
        t = time_passed / Duration();
    spline.computeIndex(t, seg_Idx, u);
    return _ComputePosition(seg_Idx, u);
}

Vector4 MoveSpline::_ComputePosition(SplineBase::index_type seg_Idx, float u) const
{
    mov_assert(Initialized());

    Vector4 c;
    Vector3 hermite;
    spline.evaluate_percent(seg_Idx,u, (Vector3&)c);
    spline.evaluate_hermite(seg_Idx,u, hermite);

    if (splineflags.animation)
        ;// SPLINEFLAG_ANIMATION disables falling or parabolic movement
    else if (splineflags.parabolic)
        computeParabolicElevation(c.z);
    else if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags.done && (splineflags & MoveSplineFlag::Mask_Final_Facing))
    {
        if (splineflags.final_angle)
            c.w = facing.angle;
        else if (splineflags.final_point)
            c.w = G3D::wrap(atan2(facing.spot.y-c.y, facing.spot.x-c.x), 0.f, (float)G3D::twoPi());
        //nothing to do for SPLINEFLAG_FINAL_TARGET flag
    }
    else
        c.w = G3D::wrap(atan2(hermite.y, hermite.x), 0.f, (float)G3D::twoPi());
    return c;
}

void MoveSpline::computeParabolicElevation(float& el) const
{
    if (time_passed > spec_effect_time)
    {
        float t_passedf = MSToSec(time_passed - spec_effect_time);
        float t_durationf = MSToSec(Duration() - spec_effect_time); //client use not modified duration here

        // -a*x*x + bx + c:
        //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
        el += (t_durationf - t_passedf) * 0.5f * vertical_acceleration * t_passedf;
    }
}

void MoveSpline::computeFallElevation(float& el) const
{
    float z_now = spline.getPoint(spline.first()).z - Movement::computeFallElevation(MSToSec(time_passed), false, 0.f);

    if (z_now < finalDestination.z)
    {
        el = finalDestination.z;
        log_write("MoveSpline::ComputePosition: z_now < finalDestination.z");
    }
    else
        el = z_now;
}

inline uint32 computeDuration(float length, float velocity)
{
    return SecToMS(length / velocity);
}

void MoveSpline::Initialize(const MoveSplineInitArgs& args)
{
    mov_assert(args.Validate());

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

    const SplineBase::EvaluationMode modes[2] = {SplineBase::ModeLinear,SplineBase::ModeCatmullrom};

    if (isCyclic())
    {
        uint32 cyclic_point = 0;
        // SPLINEFLAG_ENTER_CYCLE support dropped
        //if (splineflags & SPLINEFLAG_ENTER_CYCLE)
            //cyclic_point = 1;   // shouldn't be modified, came from client

        spline.init_cyclic_spline(&args.path[0], args.path.size(), modes[isSmooth()], 1000.f / args.velocity, cyclic_point);
        finalDestination = Vector3::zero();

        int32 duration = spline.length(spline.first()+cyclic_point,spline.last());
        mov_assert(duration != 0 && "posssible division by zero in OnArrived call");
    }
    else
    {
        spline.init_spline(&args.path[0], args.path.size(), modes[isSmooth()], 1000.f / args.velocity);

        if (splineflags.falling)
        {
            class SplineExt : private MySpline
            {
            public:
                void modify_lengths(float coeff)
                {
                    index_type i = first();
                    while(i <= last())
                        lengths[i++] *= coeff;
                }
            };

            float fall_time_ms = 1000.f * computeFallTime(args.path[0].z - finalDestination.z, false);
            ((SplineExt&)spline).modify_lengths( spline.length() / fall_time_ms );
        }

        finalDestination = spline.getPoint(spline.last());
    }

    // path initialized, duration is known and i able to compute parabolic acceleration
    if (splineflags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
    {
        spec_effect_time = Duration() * args.time_perc;
        if (splineflags.parabolic)
        {
            float f_duration = MSToSec(Duration() - spec_effect_time);
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
    str << "time passed:  " << time_passed << std::endl;
    str << "total time:   " << Duration() << std::endl;
    str << spline.ToString();
    return str.str();
}

MoveSpline::MoveSpline() : m_Id(MoveSplineCounter::Lower_limit), time_passed(0),
    vertical_acceleration(0.f), spec_effect_time(0)
{
}

void MoveSpline::OnArrived()
{
    if (isCyclic())
    {
        time_passed = time_passed % Duration();

        // SPLINEFLAG_ENTER_CYCLE support dropped
        /*if (splineflags & SPLINEFLAG_ENTER_CYCLE)
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
        }*/

        //duration_mod = this->duration_mod_next;
        //duration_mod_next = 1.f;
        //duration_ = modifiedDuration();
    }
    else
    {
        Finalize();
        time_passed = Duration();
    }
}

/// ============================================================================================

bool MoveSplineInitArgs::Validate() const
{
    return path.size() > 1 && velocity > 0.f &&
        time_perc >= 0.f && time_perc <= 1.f && _checkPathBounds();
}

// MONSTER_MOVE packet format limitation for not CatmullRom movement:
// extent of path vertices should fit inside 255x255x255 bounding box
bool MoveSplineInitArgs::_checkPathBounds() const
{
    if (!(flags & MoveSplineFlag::Mask_CatmullRom) && path.size() > 2)
    {
        Vector3 middle = (path.front()+path.back()) / 2;
        Vector3 offset;
        for (uint32 i = 1; i < path.size()-1; ++i)
        {
            offset = path[i] - middle;
            if (fabs(offset.x) >= 255 || fabs(offset.y) >= 255 || fabs(offset.z) >= 255)
            {
                log_console("MoveSplineInitArgs::_checkPathBounds check failed");
                return false;
            }
        }
    }
    return true;
}

/// ============================================================================================

MoveSpline::UpdateResult MoveSplineSegmented::_updateState(int32 ms_time_diff)
{
    mov_assert(Initialized());
    mov_assert(ms_time_diff >= 0);

    if (Finalized())
        return Result_StopUpdate;

    UpdateResult result = Result_None;
    time_passed += ms_time_diff;

    if (time_passed >= segment_timestamp())
    {
        //On next segment
        ++point_Idx;

        if (point_Idx < spline.last())
        {
            result = Result_NextSegment;
        }
        else if (spline.isCyclic())
        {
            point_Idx = spline.first();
            time_passed = time_passed % Duration();

            result = Result_NextSegment;
        }
        else
        {
            Finalize();
            point_Idx = spline.last() - 1;
            time_passed = Duration();
            result = (UpdateResult)(Result_Arrived | Result_StopUpdate);
        }
    }

    return result;
}

MoveSpline::UpdateResult MoveSplineSegmented::updateState(int32& ms_time_diff)
{
    int32 minimal_diff = std::min(ms_time_diff, segment_timestamp() - time_passed);

    UpdateResult result = _updateState(minimal_diff);

    ms_time_diff -= minimal_diff;
    if (ms_time_diff <= 0)
        (uint32&)result |= Result_StopUpdate;

    return result;
}

G3D::Vector4 MoveSplineSegmented::ComputePosition() const
{
    float u = 1.f;

    int32 seg_time = spline.length(point_Idx,point_Idx+1);
    if (seg_time > 0)
        u = (time_passed - spline.length(point_Idx)) / (float)seg_time;

    return _ComputePosition(point_Idx, u);
}

void MoveSplineSegmented::Initialize(const MoveSplineInitArgs& args)
{
    MoveSpline::Initialize(args);

    point_Idx_offset = args.path_Idx_offset;
    point_Idx = spline.first();
}

MoveSplineSegmented::MoveSplineSegmented() : point_Idx(0), point_Idx_offset(0)
{
}

std::string MoveSplineSegmented::ToString() const
{
    std::stringstream str;

    str << "MoveSplineSegmented" << std::endl;
    str << "spline Id:    " << GetId() << std::endl;
    //str << "spline flags: " << print_flags(GetSplineFlags(),g_SplineFlags_names);
    str << "flags:        " << GetSplineFlags() << std::endl;
    str << "time passed:  " << time_passed << std::endl;
    str << "total time:   " << Duration() << std::endl;
    str << "segment Idx:  " << point_Idx << std::endl;
    str << spline.ToString();
    return str.str();
}

}
