
#include "UnitMovement.h"
#include "WorldPacket.h"
#include "Object.h"

#include <assert.h>

// seems only MSVC has this include, what about rest of compilers and platforms?
#include <float.h>

namespace Movement{



float MovementState::CalculateCurrentSpeed( bool is_walking /*= false*/ ) const
{
    // g_moveFlags_mask - some global client's moveflag mask
    // TODO: get real value
    static uint32 g_moveFlags_mask = 0;

    //if ( !(g_moveFlags_mask & moveFlags) )
        //return 0.0f;

    if ( moveFlags & MOVEFLAG_FLYING )
    {
        if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.flight >= speed_obj.flight_back )
            return speed_obj.flight_back;
        else
            return speed_obj.flight;
    }
    else if ( moveFlags & MOVEFLAG_SWIMMING )
    {
        if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.swim >= speed_obj.swim_back )
            return speed_obj.swim_back;
        else
            return speed_obj.swim;
    }
    else
    {
        if ( moveFlags & MOVEFLAG_WALK_MODE || is_walking )
        {
            if ( speed_obj.run > speed_obj.walk )
                return speed_obj.walk;
        }
        else
        {
            if ( moveFlags & MOVEFLAG_BACKWARD && speed_obj.run >= speed_obj.run_back )
                return speed_obj.run_back;
        }
        return speed_obj.run;
    }
}

void MovementState::ApplyMoveMode( MoveMode mode, bool apply )
{
    if (apply)
    {
        AddMovementFlag(Mode2Flag_table[mode]);
        move_mode |= (1 << mode);
    }
    else
    {
        RemoveMovementFlag(Mode2Flag_table[mode]);
        move_mode &= ~(1 << mode);
    }
}

MovementState::MovementState(WorldObject * owner) : UnitBase(*owner), msg_builder(*this, MovControlServer)
{
    move_mode = 0;
    last_ms_time = 0;
    moveFlags = 0;
    move_flags2 = 0;

    memcpy(&speed, BaseSpeed, sizeof BaseSpeed);

    s_pitch = 0.f;
    // last fall time
    fallTime = 0;
    fallStartElevation = 0.f;
    // jumping
    j_velocity = j_sinAngle = j_cosAngle = j_xy_velocy = 0.f;

    u_unk1 = 0.f;
}

void MovementState::ReCalculateCurrentSpeed()
{
    speed_obj.current = CalculateCurrentSpeed(false);
}

void MovementState::Initialize( MovControlType controller, const Vector4& pos, uint32 ms_time )
{
    last_ms_time = ms_time;

    SetPosition(pos);
    GetBuilder().SetControl(controller);
}


// for debugging:
// there were problems with NaN coords in past
inline bool _finiteV(const Vector3& v)
{
    return _finite(v.x) && _finite(v.y) && _finite(v.z);
}

void MovementState::SetPosition(const Vector4& v)
{
    if (!_finiteV((Vector3&)v))
    {
        log_write("MovementState::SetPosition: NaN coord detected");
        return;
    }

    position = v;
}

void MovementState::SetPosition(const Vector3& v)
{
    if (!_finiteV(v))
    {
        log_write("MovementState::SetPosition: NaN coord detected");
        return;
    }

    (Vector3&)position = v;
}

void SplineFace::ResetSplineState()
{
    if (SplineEnabled())
    {
        UpdateState();



        DisableSpline();
        ResetDirection();

        // TODO: should we send packet directly from here?
        SendPath();
    }
}

void SplineFace::UpdateState()
{
    if (SplineEnabled())
    {
        uint32 now = getMSTime();
        move_spline.updateState(now);
        SetPosition(move_spline.ComputePosition(), now);

        if (move_spline.Finalized())
        {
            DisableSpline();
            ResetDirection();

            if (listener)
                listener->OnSplineDone();
        }
    }
}

void SplineFace::SendPath()
{
    WorldPacket data;
    GetBuilder().PathUpdate(data);
    m_owner.SendMessageToSet(&data, true);
}

MoveSplineInit& MoveSplineInit::MovebyPath( const PointsArray& controls, bool is_cyclic )
{
    if (is_cyclic)
        spline.splineflags |= SPLINEFLAG_CYCLIC | SPLINEFLAG_ENTER_CYCLE;
    else
        spline.splineflags &= ~(SPLINEFLAG_CYCLIC | SPLINEFLAG_ENTER_CYCLE);

    m_path.resize(controls.size() + 1);
    memcpy(&m_path[1], &controls[0], sizeof(PointsArray::value_type) * controls.size());
    
    return *this;
}

MoveSplineInit& MoveSplineInit::MoveTo( const Vector3& dest )
{
    m_path.resize(2);
    m_path[1] = dest;
    return *this;
}

MoveSplineInit& MoveSplineInit::MoveFall( const Vector3& dest )
{
    spline.splineflags = SPLINEFLAG_FALLING;

    m_path.resize(2);
    m_path[1] = dest;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFly()
{
    spline.splineflags |= SPLINEFLAG_FLYING;
    spline.splineflags &= ~SPLINEFLAG_CATMULLROM;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetWalk()
{
    spline.splineflags |= SPLINEFLAG_WALKMODE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetSmooth()
{
    spline.splineflags |= SPLINEFLAG_CATMULLROM;
    spline.splineflags &= ~SPLINEFLAG_FLYING;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetVelocity( float vel )
{
    velocity = vel;
    return *this;
}

void MoveSplineInit::Apply()
{
    // update previous state first
    if (state.SplineEnabled())
        state.GetSplineFace().UpdateState();

    assert(m_path.size() >= 2);

    if (velocity != 0.f)
        state.speed_obj.current = velocity;
    else
        state.ReCalculateCurrentSpeed();

    m_path[0] = state.GetPosition3();

        spline.partial_initialize(m_path, state.speed_obj.current, max_vertical_height);

    state.move_spline = spline;

    state.EnableSpline();
    state.SetForwardDirection();
}

MoveSplineInit& MoveSplineInit::SetTrajectory( float max_height, uint32 time_shift )
{
    spline.splineflags |= SPLINEFLAG_TRAJECTORY;
    spline.splineflags &= ~(SPLINEFLAG_KNOCKBACK | SPLINEFLAG_ANIMATION);
    spline.parabolic_time = time_shift;
    max_vertical_height = max_height;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetKnockBack( float max_height, uint32 time_shift )
{
    SetTrajectory(max_height, time_shift);
    spline.splineflags |= SPLINEFLAG_KNOCKBACK;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( uint64 guid )
{
    spline.facing_target = guid;
    spline.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    spline.splineflags |= SPLINEFLAG_FINAL_TARGET;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( float o )
{
    spline.facing_angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
    spline.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    spline.splineflags |= SPLINEFLAG_FINAL_ANGLE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( Vector3 const& spot )
{
    spline.facing_spot.x = spot.x;
    spline.facing_spot.y = spot.y;
    spline.facing_spot.z = spot.z;
    spline.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    spline.splineflags |= SPLINEFLAG_FINAL_POINT;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetAnimation(AnimType anim, uint32 anim_time)
{
    spline.splineflags &= ~(SPLINEFLAG_TRAJECTORY|SPLINEFLAG_KNOCKBACK);
    spline.splineflags |= SPLINEFLAG_ANIMATION;
    spline.animationType = uint8(anim);
    spline.animationTime = anim_time;
    return *this;
}

}
