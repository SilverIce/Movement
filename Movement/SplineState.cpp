#include "mov_constants.h"
#include "SplineState.h"

namespace Movement{

void MoveSpline::SetFacing( uint64 guid )
{
    facing_target = guid;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALTARGET);
}

void MoveSpline::SetFacing( float o )
{
    facing_angle = o;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALFACING);
}

void MoveSpline::SetFacing( Vector3 const& spot )
{
    facing_spot.x = spot.x;
    facing_spot.y = spot.y;
    facing_spot.z = spot.z;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALPOINT);
}

void MoveSpline::ResetFacing()
{
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
}

MoveSpline::MoveSpline()
{
    splineflags = 0;

    time_passed = 0;
}

void ParabolicHandler::handleParabolic( uint32 t_duration, uint32 t_passed, Vector3& position ) const
{
    float t_passedf = ToSeconds(t_passed - parabolic_time_shift);
    float t_durationf = ToSeconds(t_duration - parabolic_time_shift);

    // -a*x*x + bx + c:
    //(dur * v3->z_acceleration * dt)/2 - (v3->z_acceleration * dt * dt)/2 + Z;
    //position.z += t_durationf/2 * (z_acceleration * t_passedf) - (z_acceleration * t_passedf * t_passedf)/2;
    position.z += (t_durationf - t_passedf) * 0.5f * z_acceleration * t_passedf;
}

void SplineHandler::handleSpline( uint32 t_passed, Vector3& c )
{
    uint32 duration_ = (float)duration * duration_mod + 0.5f;
    time_passed += t_passed;

    if (time_passed >= duration_)
    {
        if (cyclic)
            time_passed = time_passed % duration_;
        else
        {
            time_passed = duration_;
        }
    }

    float t = (float)time_passed / (float)duration_;
    evaluate_percent(t, c);
}
}
