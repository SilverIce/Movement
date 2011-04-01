
#include "MoveSpline.h"
#include "mov_constants.h"
#include <sstream>

namespace Movement{


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

Location MoveSpline::ComputePosition() const
{
    SplineBase::index_type seg_Idx;
    float u, t = 1.f;
    if (Duration())
        t = time_passed / Duration();
    spline.computeIndex(t, seg_Idx, u);
    return _ComputePosition(seg_Idx, u);
}

Location MoveSpline::_ComputePosition(SplineBase::index_type seg_Idx, float u) const
{
    mov_assert(Initialized());

    Location c;
    spline.evaluate_percent(seg_Idx,u, c);

    if (splineflags.animation)
        ;// MoveSplineFlag::Animation disables falling or parabolic movement
    else if (splineflags.parabolic)
        computeParabolicElevation(c.z);
    else if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags & (MoveSplineFlag::Done | MoveSplineFlag::Mask_Final_Facing))
    {
        if (splineflags.final_angle)
            c.orientation = facing.angle;
        else if (splineflags.final_point)
            c.orientation = atan2(facing.y-c.y, facing.x-c.x);
        //nothing to do for MoveSplineFlag::Final_Target flag
    } 
    else
    {
        Vector3 hermite;
        spline.evaluate_hermite(seg_Idx,u,hermite);
        c.orientation = atan2(hermite.y, hermite.x);

        if (splineflags.backward)
            c.orientation = -c.orientation;
    }
    return c;
}

void MoveSpline::computeParabolicElevation(float& el) const
{
    if (time_passed > effect_start_time)
    {
        float t_passedf = MSToSec(time_passed - effect_start_time);
        float t_durationf = MSToSec(Duration() - effect_start_time); //client use not modified duration here

        // -a*x*x + bx + c:
        //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
        el += (t_durationf - t_passedf) * 0.5f * vertical_acceleration * t_passedf;
    }
}

void MoveSpline::computeFallElevation(float& el) const
{
    float z_now = spline.getPoint(spline.first()).z - Movement::computeFallElevation(MSToSec(time_passed));

    if (z_now < finalDestination.z)
    {
        el = finalDestination.z;
        log_write("MoveSpline::computeFallElevation: z_now < finalDestination.z");
    }
    else
        el = z_now;
}

inline uint32 computeDuration(float length, float velocity)
{
    return SecToMS(length / velocity);
}

struct FallInitializer
{
    FallInitializer(float _start_elevation) : start_elevation(_start_elevation) {}
    float start_elevation;
    inline int32 operator()(Spline<int32>& s, int32 i)
    {
        return Movement::computeFallTime(start_elevation - s.getPoint(i+1).z,false) * 1000.f;
    }
};

enum{
    minimal_duration = 1,
};

struct CommonInitializer
{
    CommonInitializer(float _velocity) : velocityInv(1000.f/_velocity), time(minimal_duration) {}
    float velocityInv;
    int32 time;
    inline int32 operator()(Spline<int32>& s, int32 i)
    {
        time += (s.SegLength(i) * velocityInv);
        return time;
    }
};

void MoveSpline::init_spline(const MoveSplineInitArgs& args)
{
    const SplineBase::EvaluationMode modes[2] = {SplineBase::ModeLinear,SplineBase::ModeCatmullrom};
    if (args.flags.cyclic)
    {
        uint32 cyclic_point = 0;
        // MoveSplineFlag::Enter_Cycle support dropped
        //if (splineflags & SPLINEFLAG_ENTER_CYCLE)
        //cyclic_point = 1;   // shouldn't be modified, came from client
        spline.init_cyclic_spline(&args.path[0], args.path.size(), modes[args.flags.isSmooth()], cyclic_point);
        finalDestination = Vector3::zero();
    }
    else
    {
        spline.init_spline(&args.path[0], args.path.size(), modes[args.flags.isSmooth()]);
        finalDestination = spline.getPoint(spline.last());
    }

    // init spline timestamps
    if (splineflags.falling)
        spline.initLengths( FallInitializer(spline.getPoint(spline.first()).z) );
    else
        spline.initLengths(CommonInitializer(args.velocity));

    // TODO: what to do in such cases? problem is in input data (all points are at same coords)
    // critical only for cyclic movement
    if (spline.isCyclic() && spline.length() <= minimal_duration)
    {
        log_write("MoveSpline::init_spline: Zero length spline");
        spline.set_length(spline.last(), 1000);
    }
}

void MoveSpline::Initialize(const MoveSplineInitArgs& args)
{
    mov_assert(args.Validate());

    splineflags = args.flags;
    facing = args.facing;
    m_Id = args.splineId;

    time_passed = 0;
    vertical_acceleration = 0.f;
    effect_start_time = 0;

    init_spline(args);

    // init parabolic / animation
    // spline initialized, duration known and i able to compute parabolic acceleration
    if (args.flags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
    {
        effect_start_time = Duration() * args.time_perc;
        if (args.flags.parabolic && effect_start_time < Duration())
        {
            float f_duration = MSToSec(Duration() - effect_start_time);
            vertical_acceleration = args.parabolic_heigth * 8.f / (f_duration * f_duration);
        }
    }
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

MoveSpline::MoveSpline() : m_Id(0), time_passed(0),
    vertical_acceleration(0.f), effect_start_time(0)
{
}

void MoveSpline::OnArrived()
{
    if (isCyclic())
    {
        time_passed = time_passed % Duration();

        // SPLINEFLAG_ENTER_CYCLE support dropped
        /*if (splineflags.enter_cycle)
        {
            int32 duration = Duration();
            MySpline::ControlArray points(spline.getPoints());
            spline.init_cyclic_spline(&points[spline.first()+1],points_count-1,spline.mode(),0);

            splineflags.enter_cycle = false;

            // client-side bug: client resets parabolic info to default values, but doesn't removes parabolic/trajectory flag
            // in simple words: parabolic movement can be used with cyclic movement but there will be little visual bug on client side
            // i decided remove remove trajectory flag
            splineflags.parabolic = false;
        }*/
    }
    else
    {
        Finalize();
        time_passed = Duration();
    }
}

/// ============================================================================================
extern float terminalVelocity;

bool MoveSplineInitArgs::Validate() const
{
#define CHECK(exp) \
    if (!(exp))\
    {\
        log_write("MoveSplineInitArgs::Validate: '%s' failed", #exp);\
        return false;\
    }
    CHECK(path.size() > 1);
    CHECK(velocity > 0.f && velocity <= terminalVelocity);
    CHECK(time_perc >= 0.f && time_perc <= 1.f);
    CHECK(_checkPathBounds());
    return true;
#undef CHECK
}

// MONSTER_MOVE packet format limitation for not CatmullRom movement:
// each vertex offset packed into 4 bytes and it should fit inside 255x255x255 bounding box
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

Location MoveSplineSegmented::ComputePosition() const
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
