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

SplineState::SplineState()
{
    last_ms_time = 0;
    last_positionIdx = 0;

    splineflags = 0;

    time_passed = 0;

    parabolic_speed = 0;
    parabolic_time = 0;
}

void SplineState::UpdatePosition( uint32 curr_ms_time, float velocy, Vector3 & c )
{
    // amount of time passed since last evaluate call
    uint32 t_passed = getMSTimeDiff(last_ms_time, curr_ms_time);
    last_ms_time = curr_ms_time;

    /** convert passed time to absolute passed time:
    *   if     absolute_velocy = velocy * alpha
    *   then   absolute_passed_time = t_passed / alpha;
        i.e.
        double alpha = absolute_velocy / velocy;
        t_passed = double(t_passed) / alpha;
    */
        t_passed = double(t_passed) * velocy / absolute_velocy;

    
    uint32 X = time_passed + t_passed;
    spline.evaluate(X, c);
}

void SplineState::init_path( const Vector3 * controls, const int count, SplineMode m, bool cyclic )
{
    RemoveSplineFlag(SPLINEFLAG_CATMULLROM | SPLINEFLAG_BEZIER3 | SPLINEFLAG_CYCLIC);

    if (m == SplineModeCatmullrom)
        AddSplineFlag(SPLINEFLAG_CATMULLROM);

    else if (m == SplineModeBezier3)
        AddSplineFlag(SPLINEFLAG_BEZIER3);

    if (cyclic)
        AddSplineFlag(SPLINEFLAG_CYCLIC);

    spline.init_path(path, count, m, cyclic);

    last_positionIdx = 0;
}