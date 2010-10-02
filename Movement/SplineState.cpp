#include "mov_constants.h"
#include "SplineState.h"

using namespace Movement;

void SplineState::SetFacing( uint64 guid )
{
    facing_info.target = guid;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALTARGET);
}

void SplineState::SetFacing( float o )
{
    facing_info.angle = o;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALFACING);
}

void SplineState::SetFacing( Vector3 const& spot )
{
    facing_info.spot.x = spot.x;
    facing_info.spot.y = spot.y;
    facing_info.spot.z = spot.z;
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
    AddSplineFlag(SPLINEFLAG_FINALPOINT);
}

void SplineState::ResetFacing()
{
    RemoveSplineFlag(SPLINE_MASK_FINAL_FACING);
}

Movement::SplineState::SplineState()
{
    spline_path.reserve(10);

    time_stamp = 0;
    splineflags = 0;
    total_lenght = 0;

    move_time_full = 0;
    move_time_passed = 0;

    mode = SplineModeLinear;

    parabolic_speed = 0;
    parabolic_time = 0;
}