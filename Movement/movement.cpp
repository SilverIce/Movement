
#include "movement.h"
#include "WorldPacket.h"
#include "Object.h"

// seems only MSVC has this include, what about rest of compilers and platforms?
#include <float.h>

namespace Movement{

// TODO: make it Atomic
static counter<uint32> MoveSplineCounter;

inline void NewSpline(MoveSpline& move)
{
    move.Init();
    move.sequence_Id = MoveSplineCounter.increase();
}

float MovementState::CalculateCurrentSpeed( bool is_walking /*= false*/ ) const
{
    // g_moveFlags_mask - some global client's moveflag mask
    // TODO: get real value
    static uint32 g_moveFlags_mask = 0xFFFFFFFF;
    float speed = 0.0f;

    if ( !(g_moveFlags_mask & moveFlags) )
        return 0.0f;

    if ( /*!move_spline ||*/ move_spline.splineflags & SPLINEFLAG_NO_SPLINE )
    {
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
    else
    {
        if ( !move_spline.duration )
            return 0.0f;
        speed = move_spline.spline.length() / move_spline.duration * 1000.0f;
    }
    return speed;
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

MovementState::MovementState(WorldObject * owner) : msg_builder(this, MovControlServer)
{
    m_owner = owner;

    move_mode = 0;
    last_ms_time = 0;
    moveFlags = 0;
    move_flags2 = 0;

    last_ms_time = 0;

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
    SetPosition(pos, ms_time);

    GetBuilder().SetControl(controller);
}

MoveSpline& MovementState::NewSpline()
{
    move_spline.Init();
    move_spline.sequence_Id = MoveSplineCounter.increase();

    return move_spline;
}

// for debugging:
// there were problems with NaN coords in past
inline bool _finiteV(const Vector3& v)
{
    return _finite(v.x) && _finite(v.y) && _finite(v.z);
}

void MovementState::SetPosition( const Vector4& v, uint32 ms_time )
{
    if (!_finiteV((const Vector3&)v))
    {
        movLog.write("MovementState::SetPosition: NaN coord detected");
        return;
    }

    position = v;
    last_ms_time = ms_time;
}

void MovementState::SetPosition( const Vector3& v, uint32 ms_time )
{
    if (!_finiteV(v))
    {
        movLog.write("MovementState::SetPosition: NaN coord detected");
        return;
    }

    (Vector3&)position = v;
    last_ms_time = ms_time;
}

void SplineFace::ResetSplineState()
{
    if (SplineEnabled())
    {
        UpdateState();

        MoveSpline& move = NewSpline();

        move.reset_state();

        DisableSpline();
        ResetDirection();

        // TODO: should we send packet directly from here?
        WorldPacket data;
        GetBuilder().PathUpdate(data);
        m_owner->SendMessageToSet(&data, true);
    }
}

void SplineFace::UpdateState()
{
    if (SplineEnabled())
    {
        Vector4 c;
        uint32 now = getMSTime();
        move_spline.updateState(now, c);
        //Spline_computeElevation(t_passed, c);
        SetPosition(c, now);
    }
}

void SplineFace::SendPath()
{
    WorldPacket data;
    GetBuilder().PathUpdate(data);
    m_owner->SendMessageToSet(&data, true);
}

#pragma region MoveSplineInit

MoveSplineInit::SecondInit& MoveSplineInit::MovebyPath( const PointsArray& controls )
{
    state.EnableSpline();
    state.SetForwardDirection();
    // appends current position
    PointsArray copy(controls.begin(), controls.end());
    copy.insert(copy.begin(), state.GetPosition3());
    
    NewSpline(move);
    move.init_spline(getMSTime(), copy, state.GetCurrentSpeed(), m_new_flags);
    return init2;
}

MoveSplineInit::SecondInit& MoveSplineInit::MovebyCyclicPath( const PointsArray& controls )
{
    state.EnableSpline();
    state.SetForwardDirection();
    // appends current position
    PointsArray copy(controls.begin(), controls.end());
    copy.insert(copy.begin(), state.GetPosition3());

    NewSpline(move);
    move.init_cyclic_spline(getMSTime(), copy, state.GetCurrentSpeed(), m_new_flags);
    return init2;
}

MoveSplineInit::SecondInit& MoveSplineInit::MoveTo( const Vector3& dest )
{
    state.EnableSpline();
    state.SetForwardDirection();

    PointsArray path(2);
    path[0] = state.GetPosition3();
    path[1] = dest;

    NewSpline(move);
    move.init_spline(getMSTime(), path, state.GetCurrentSpeed(), m_new_flags);
    return init2;
}

void MoveSplineInit::MoveFall( const Vector3& dest )
{
    m_new_flags = SPLINEFLAG_FALLING;

    state.ResetDirection();
    state.EnableSpline();

    PointsArray path(2);
    path[0] = state.GetPosition3();
    path[1] = dest;

    NewSpline(move);
    move.init_spline(getMSTime(), path, computeFallTime(path[0].z - path[1].z, false), m_new_flags);
}

MoveSplineInit& MoveSplineInit::SetFly()
{
    m_new_flags |= SPLINEFLAG_FLYING;
    m_new_flags &= ~SPLINEFLAG_CATMULLROM;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetWalk()
{
    m_new_flags |= SPLINEFLAG_WALKMODE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetSmooth()
{
    m_new_flags |= SPLINEFLAG_CATMULLROM;
    m_new_flags &= ~SPLINEFLAG_FLYING;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetVelocy( float velocy )
{
    state.speed_obj.current = velocy;
    return *this;
}

MoveSplineInit::SecondInit& MoveSplineInit::SecondInit::SetTrajectory( float z_velocy, uint32 time_shift )
{
    move.splineflags |= SPLINEFLAG_TRAJECTORY;
    move.parabolic.z_acceleration = z_velocy;
    move.parabolic.time_shift = time_shift;
    return *this;
}

MoveSplineInit::SecondInit& MoveSplineInit::SecondInit::SetKnockBack( float z_velocy, uint32 time_shift )
{
    move.splineflags |= SPLINEFLAG_TRAJECTORY | SPLINEFLAG_KNOCKBACK;
    move.parabolic.z_acceleration = z_velocy;
    move.parabolic.time_shift = time_shift;
    return *this;
}

MoveSplineInit::SecondInit& MoveSplineInit::SecondInit::SetFacing( uint64 guid )
{
    move.facing_target = guid;
    move.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    move.splineflags |= SPLINEFLAG_FINAL_TARGET;
    return *this;
}

MoveSplineInit::SecondInit& MoveSplineInit::SecondInit::SetFacing( float o )
{
    move.facing_angle = o;
    move.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    move.splineflags |= SPLINEFLAG_FINAL_ANGLE;
    return *this;
}

MoveSplineInit::SecondInit& MoveSplineInit::SecondInit::SetFacing( Vector3 const& spot )
{
    move.facing_spot.x = spot.x;
    move.facing_spot.y = spot.y;
    move.facing_spot.z = spot.z;
    move.splineflags &= ~SPLINE_MASK_FINAL_FACING;
    move.splineflags |= SPLINEFLAG_FINAL_POINT;
    return *this;
}

#pragma endregion


}
