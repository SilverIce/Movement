
#include "MoveSpline.h"
#include <QtCore/QTextStream>

namespace Movement{

inline uint32 SecToMS(float sec) {
    return static_cast<uint32>(sec * 1000.f);
}

inline float MSToSec(uint32 ms) {
    return ms / 1000.f;
}

Vector4 MoveSpline::ComputePosition() const
{
    float u = 1.f;
    int32 seg_time = spline.lengthBetween(point_Idx,point_Idx+1);
    if (seg_time > 0)
        u = (time_passed - spline.length(point_Idx)) / (float)seg_time;

    Vector4 c(spline.evaluatePosition(point_Idx,u), initialOrientation);

    if (splineflags.animation)
        ;// MoveSplineFlag::Animation disables falling or parabolic movement
    else if (splineflags.parabolic)
        computeParabolicElevation(c.z);
    else if (splineflags.falling)
        computeFallElevation(c.z);

    if (splineflags.done && splineflags.isFacing())
    {
        if (splineflags.final_angle)
            c.w = facing.angle;
        else if (splineflags.final_point)
            c.w = atan2f(facing.y-c.y, facing.x-c.x);
        //nothing to do for MoveSplineFlag::Final_Target flag
    }
    else
    {
        if (!splineflags.hasFlag(MoveSplineFlag::RotationFixed|MoveSplineFlag::Falling))
        {
            Vector3 direction = spline.evaluateDerivative(point_Idx,u);
            c.w = atan2f(direction.y, direction.x);
        }

        if (splineflags.orientationInversed)
            c.w = -c.w;
    }
    return c;
}

void MoveSpline::computeParabolicElevation(float& el) const
{
    if (time_passed > effect_start_time)
    {
        float t_passedf = MSToSec(time_passed - effect_start_time);
        float t_durationf = MSToSec(timeTotal() - effect_start_time);

        // -a*x*x + bx + c:
        //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
        el += (t_durationf - t_passedf) * 0.5f * vertical_acceleration * t_passedf;
    }
}

extern double computeFallElevation(float time);
extern double computeFallTime(float elevation);

void MoveSpline::computeFallElevation(float& el) const
{
    el = spline.getPoint(0).z - Movement::computeFallElevation(MSToSec(time_passed));
}

enum{
    minimal_duration = 1,
};

void MoveSpline::init_spline(const MoveSplineInitArgs& args)
{
    const SplineBase::EvaluationMode modes[2] = {SplineBase::ModeLinear,SplineBase::ModeCatmullrom};
    if (args.flags.cyclic)
    {
        uint32 cyclic_point = 0;
        // MoveSplineFlag::Enter_Cycle no more supported
        //if (splineflags & MoveSplineFlag::Enter_Cycle)
        //cyclic_point = 1;   // shouldn't be modified, came from client
        spline.initCyclicSpline(&args.path[0], args.path.size(), modes[args.flags.isSmooth()], cyclic_point);
    }
    else
    {
        spline.initSpline(&args.path[0], args.path.size(), modes[args.flags.isSmooth()]);
    }

    // init spline timestamps
    if (args.flags.done)
    {
        struct InstantInitializer {
            int32 operator()(const Spline<int32>& s, int32 i) { return minimal_duration;}
        };
        // total movement duration is 1 millisecond for now
        spline.initLengths(InstantInitializer());
    }
    else
    {
        if (splineflags.falling) {
            struct FallInitializer {
                FallInitializer(float _start_elevation) : start_elevation(_start_elevation) {}
                float start_elevation;
                int32 operator()(const Spline<int32>& s, int32 pointIdx) {
                    return Movement::computeFallTime(start_elevation - s.getPoint(pointIdx).z) * 1000.f;
                }
            };
            spline.initLengths( FallInitializer(args.path[0].z) );
        }
        else {
            struct CommonInitializer {
                CommonInitializer(float _velocity) : velocityInv(1000.f/_velocity), time(minimal_duration) {}
                float velocityInv;
                int32 time;
                int32 operator()(const Spline<int32>& s, int32 pointIdx) {
                    time += (s.segmentLength(pointIdx-1) * velocityInv);
                    return time;
                }
            };
            spline.initLengths(CommonInitializer(args.velocity));
        }
    }

    // TODO: what to do in such cases? problem is in input data (all points are at same coords)
    // critical only for cyclic movement
    if (args.flags.cyclic && spline.lengthTotal() <= minimal_duration)
    {
        log_write("MoveSpline::init_spline: Zero length spline");
        spline.lengthTotal(1000);
    }
    point_Idx = 0;
}

void MoveSpline::Initialize(const MoveSplineInitArgs& args)
{
    mov_assert(args.Validate());

    splineflags = args.flags;
    facing = args.facing;
    m_Id = args.splineId;
    point_Idx_offset = args.path_Idx_offset;
    initialOrientation = args.initialOrientation;

    time_passed = 0;
    vertical_acceleration = 0.f;
    effect_start_time = 0;

    init_spline(args);

    // init parabolic / animation
    // spline initialized, duration known and i able to compute parabolic acceleration
    if (args.flags & (MoveSplineFlag::Parabolic | MoveSplineFlag::Animation))
    {
        effect_start_time = timeTotal() * args.time_perc;
        if (args.flags.parabolic && effect_start_time < timeTotal())
        {
            float f_duration = MSToSec(timeTotal() - effect_start_time);
            vertical_acceleration = args.parabolic_amplitude * 8.f / (f_duration * f_duration);
        }
    }
}

MoveSpline::MoveSpline() : m_Id(0), time_passed(0),
    vertical_acceleration(0.f), effect_start_time(0), point_Idx(0), point_Idx_offset(0), initialOrientation(0.f)
{
}

/// ============================================================================================

float VelocityLimit = 200.f;

bool MoveSplineInitArgs::Validate() const
{
#define CHECK(exp) \
    if (!(exp))\
    {\
        log_write("MoveSplineInitArgs::Validate: expression '%s' failed", #exp);\
        return false;\
    }
    CHECK(path.size() > 1);
    CHECK(velocity > 0.f && velocity < VelocityLimit);
    CHECK(time_perc >= 0.f && time_perc <= 1.f);
    return true;
#undef CHECK
}

/// ============================================================================================

MoveSpline::UpdateResult MoveSpline::_updateState(int32& ms_time_diff)
{
    if (Arrived())
    {
        ms_time_diff = 0;
        return Result_Arrived;
    }

    UpdateResult result = Result_None;

    int32 minimal_diff = std::min(ms_time_diff, segment_time_elapsed());
    mov_assert(minimal_diff >= 0);
    time_passed += minimal_diff;
    ms_time_diff -= minimal_diff;

    if (time_passed >= timeInNextPoint())
    {
        ++point_Idx;
        if (point_Idx < spline.last())
        {
            result = Result_NextSegment;
        }
        else
        {
            if (isCyclic())
            {
                point_Idx = 0;
                time_passed = time_passed % timeTotal();
                result = Result_NextSegment;
            }
            else
            {
                Finalize();
                ms_time_diff = 0;
                result = Result_Arrived;
            }
        }
    }

    return result;
}

void MoveSpline::toString(QTextStream& str) const
{
    str << endl << "MoveSpline";
    str << endl << "spline Id: " << GetId();
    str << endl << "flags: " << splineflags.toString();
    str << endl;
    if (splineflags.final_angle)
        str << "facing  angle: " << facing.angle;
    else if (splineflags.final_target)
        str << "facing target: " << facing.target;
    else if(splineflags.final_point)
        str << "facing  point: " << facing.x << " " << facing.y << " " << facing.z;
    str << endl << "time passed: " << time_passed;
    str << endl << "total  time: " << timeTotal();
    str << endl << "spline point Id: " << point_Idx;
    str << endl << "path  point  Id: " << currentPathPointIdx();
    str << spline.toString();
}

void MoveSpline::Finalize()
{
    splineflags.done = true;
    point_Idx = spline.last() - 1;
    time_passed = timeTotal();
}

int32 MoveSpline::currentPathPointIdx() const
{
    int32 point = point_Idx_offset + point_Idx + (int32)Arrived();
    if (isCyclic())
        point = point % (spline.last()/*-spline.first()*/);
    return point;
}
}
