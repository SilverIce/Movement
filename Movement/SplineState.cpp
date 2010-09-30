#include "client_constants.h"
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