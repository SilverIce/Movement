
#include "UnitMovement.h"
#include "WorldPacket.h"
#include "Object.h"


namespace Movement{

SpeedType MovementState::SelectSpeedType( bool is_walking /*= false*/ ) const
{
    // g_moveFlags_mask - some global client's moveflag mask
    // TODO: get real value
    static uint32 g_moveFlags_mask = 0;

    //if ( !(g_moveFlags_mask & moveFlags) )
        //return 0.0f;

    if ( moveFlags & MOVEFLAG_FLYING )
    {
        if ( moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.flight >= speed_obj.flight_back*/ )
            return SpeedFlightBack;
        else
            return SpeedFlight;
    }
    else if ( moveFlags & MOVEFLAG_SWIMMING )
    {
        if ( moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.swim >= speed_obj.swim_back*/ )
            return SpeedSwimBack;
        else
            return SpeedSwim;
    }
    else
    {
        if ( moveFlags & MOVEFLAG_WALK_MODE || is_walking )
        {
            //if ( speed_obj.run > speed_obj.walk )
                return SpeedWalk;
        }
        else
        {
            if ( moveFlags & MOVEFLAG_BACKWARD /*&& speed_obj.run >= speed_obj.run_back*/ )
                return SpeedRunBack;
        }
        return SpeedRun;
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
    speed_obj.current = BaseSpeed[SpeedRun];

    s_pitch = 0.f;
    // last fall time
    fallTime = 0;
    fallStartElevation = 0.f;
    // jumping
    j_velocity = j_sinAngle = j_cosAngle = j_xy_velocy = 0.f;

    u_unk1 = 0.f;
    speed_type = SpeedRun;
}

void MovementState::ReCalculateCurrentSpeed()
{
    speed_type = SelectSpeedType(false);
    speed_obj.current = speed[speed_type];
}

void MovementState::Initialize( MovControlType controller, const Vector4& pos, uint32 ms_time )
{
    last_ms_time = ms_time;

    SetPosition(pos);
    GetBuilder().SetControl(controller);
}


{
}

{
    {

}

{
    {
    }

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

{

MoveSplineInit& MoveSplineInit::MovebyPath( const PointsArray& controls )
{
    path.resize(controls.size() + 1);
    memcpy(&path[1], &controls[0], sizeof(PointsArray::value_type) * controls.size());
    return *this;
}

MoveSplineInit& MoveSplineInit::MoveTo( const Vector3& dest )
{
    path.resize(2);
    path[1] = dest;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFly()
{
    flags |= SPLINEFLAG_FLYING;
    flags &= ~SPLINEFLAG_CATMULLROM;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetWalk()
{
    flags |= SPLINEFLAG_WALKMODE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetSmooth()
{
    flags |= SPLINEFLAG_CATMULLROM;
    flags &= ~SPLINEFLAG_FLYING;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetCyclic()
{
    flags |= SPLINEFLAG_CYCLIC | SPLINEFLAG_ENTER_CYCLE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFall()
{
    flags |= SPLINEFLAG_FALLING;
    flags &= ~SPLINEFLAG_TRAJECTORY;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetVelocity( float vel )
{
    velocity = vel;
    return *this;
}

void MoveSplineInit::Launch()
{
    // update previous state first
    if (state.SplineEnabled())
        state.GetSplineFace().UpdateState();


    if (velocity != 0.f)
    {
        state.speed_obj.current = velocity;
        state.speed_type = SpeedNotStandart;
    }
    else
    {
        state.ReCalculateCurrentSpeed();
        velocity = state.GetCurrentSpeed();
    }

    // no sense to move unit
    // TODO: find more elegant way (maybe just set current_speed to some minimal value)
    if (velocity > 0.f)
    {
        path[0] = state.GetPosition3();

        MoveSpline& spline = state.move_spline;
        spline.Initialize(*this);

        state.EnableSpline();
        state.SetForwardDirection();


    }
}

MoveSplineInit& MoveSplineInit::SetTrajectory( float max_height, float time_shift )
{
    flags |= SPLINEFLAG_TRAJECTORY;
    flags &= ~(SPLINEFLAG_KNOCKBACK | SPLINEFLAG_ANIMATION);
    time_perc = time_shift;
    parabolic_heigth = max_height;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetKnockBack( float max_height, float time_shift )
{
    SetTrajectory(max_height, time_shift);
    flags |= SPLINEFLAG_KNOCKBACK;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( uint64 t )
{
    facing.target = t;
    flags &= ~SPLINE_MASK_FINAL_FACING;
    flags |= SPLINEFLAG_FINAL_TARGET;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( float o )
{
    facing.angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
    flags &= ~SPLINE_MASK_FINAL_FACING;
    flags |= SPLINEFLAG_FINAL_ANGLE;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( Vector3 const& spot )
{
    facing.spot.x = spot.x;
    facing.spot.y = spot.y;
    facing.spot.z = spot.z;
    flags &= ~SPLINE_MASK_FINAL_FACING;
    flags |= SPLINEFLAG_FINAL_POINT;
    return *this;
}

MoveSplineInit::MoveSplineInit(MovementState& m) : state(m)
{
}

MoveSplineInit& MoveSplineInit::SetAnimation(AnimType anim, float anim_time)
{
    flags &= ~(SPLINEFLAG_TRAJECTORY|SPLINEFLAG_KNOCKBACK);
    flags = flags & ~0xFF | uint8(anim) | SPLINEFLAG_ANIMATION;
    time_perc = anim_time;
    return *this;
}

}
