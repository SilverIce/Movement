
#include "movement.h"

namespace Movement{


float MovementState::computeFallElevation( float t_passed, bool _boolean, float start_velocy )
{
    double termVel;
    double result;

    if ( _boolean )
        termVel = terminalVelocity;
    else
        // TODO: there should be another terminal velocy
        termVel = terminalVelocity;

    if ( start_velocy > termVel )
        start_velocy = termVel;

    if ( gravity * t_passed + start_velocy > termVel )
    {
        double dvel = (termVel - start_velocy);
        result = termVel * (t_passed - dvel/gravity) + (start_velocy * dvel  +  0.5 * dvel * dvel) / gravity;
    }
    else
        result = t_passed * (start_velocy + t_passed * gravity / 2);

    return result;
}

/*
void MovementState::Spline_computeElevation( uint32 t_passed, Vector3& position )
{
    if (splineInfo.HasSplineFlag(SPLINEFLAG_TRAJECTORY))
    {
        splineInfo.parabolic_h(splineInfo.duration, splineInfo.spline_h.time_passed, position);
        return;
    }

    if (splineInfo.HasSplineFlag(SPLINEFLAG_FALLING))
    {
        float z_now = fallStartElevation - MovementState::computeFallElevation(t_passed, false, 0.f);

        if (z_now < splineInfo.finalDestination.z)
            position.z = splineInfo.finalDestination.z;
        else
            position.z = z_now;
    }
}*/


float MovementState::CalculateCurrentSpeed( bool is_walking /*= false*/ ) const
{
    uint32 splineflags = splineInfo.splineflags;

    // g_moveFlags_mask - some global client's moveflag mask
    // TODO: get real value
    static uint32 g_moveFlags_mask = 0xFFFFFFFF;
    float speed = 0.0f;

    if ( !(g_moveFlags_mask & moveFlags) )
        return 0.0f;

    if ( /*!splineInfo ||*/ splineflags & SPLINEFLAG_NO_SPLINE )
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
        if ( !splineInfo.duration )
            return 0.0f;
        speed = splineInfo.length() / splineInfo.duration * 1000.0f;
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

void MovementState::MovebyPath( const Vector3* controls, int count, float speed, bool cyclic )
{
    speed_obj.current = speed;
    MovementState::MovebyPath(controls,count,cyclic);
}

void MovementState::MovebyPath( const Vector3*controls, int count, bool cyclic )
{
    AddMovementFlag(MOVEFLAG_SPLINE_ENABLED);
    splineInfo.init_path(controls, count, speed_obj.current, cyclic);
}

void MovementState::UpdatePosition( uint32 curr_ms_time, Vector3 & c )
{
    if (GetBuilder().GetControl() == MovControlClient)
        return;

    // amount of time passed since last update call
    uint32 t_passed = getMSTimeDiff(last_ms_time, curr_ms_time);
    last_ms_time = curr_ms_time;

    if (HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
        splineInfo.handleSpline(t_passed, c);

    if (splineInfo.HasSplineFlag(SPLINEFLAG_TRAJECTORY))
        splineInfo.handleParabolic(splineInfo.duration, splineInfo.time_passed, c);

    //Spline_computeElevation(t_passed, c);
}

void MovementState::UpdatePositionWithTickDiff( uint32 t_passed, Vector3 & c )
{
    if (GetBuilder().GetControl() == MovControlClient)
        return;

    // amount of time passed since last update call
    //last_ms_time = getMSTime();

    if (HasMovementFlag(MOVEFLAG_SPLINE_ENABLED))
        splineInfo.handleSpline(t_passed, c);

    if (splineInfo.HasSplineFlag(SPLINEFLAG_TRAJECTORY))
        splineInfo.handleParabolic(splineInfo.duration, splineInfo.time_passed, c);

    //Spline_computeElevation(t_passed, c);
}

void MovementState::Initialize( MovControlType controller, Vector4& pos, uint32 ms_time )
{
    last_ms_time = ms_time;

    position = pos;

    GetBuilder().SetControl(controller);
}

}