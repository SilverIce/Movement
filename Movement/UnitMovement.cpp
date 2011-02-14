
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

MovementState::MovementState(WorldObject * owner) : UnitBase(*owner)
{
    control_mode = MovControlServer;
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
    SetPosition(pos);

    last_ms_time = ms_time;
    control_mode = controller;
}

void MovementState::updateRotation(/*uint32 ms_time_diff*/)
{
    if (!IsOrientationBinded())
        return;

    const Vector3& t_pos = GetTarget()->GetPosition3();

    position.w = G3D::wrap(atan2(t_pos.y - position.y, t_pos.x - position.x), 0.f, (float)G3D::twoPi());

    // code below calculates facing angle base on turn speed, but seems this not needed:
    // server-side rotation have instant speed, i.e. units are everytimes facing to their targets
/*
    float limit_angle = G3D::wrap(atan2(t_pos.y - position.y, t_pos.x - position.x), 0.f, (float)G3D::twoPi());
    float total_angle_diff = fabs(position.w - limit_angle);

    if (total_angle_diff > 10.f/180.f * G3D::pi())
    {
        float passed_angle_diff = ms_time_diff / 1000.f * speed_obj.turn;
        passed_angle_diff = std::min(passed_angle_diff, total_angle_diff);

        position.w += passed_angle_diff;

        if (position.w > G3D::twoPi())
            position.w -= G3D::twoPi();
    }
*/
}

void MovementState::BindOrientationTo(MovementBase& target)
{
    UnitBase::BindOrientationTo(target);
    GetOwner().SetUInt64Value(UNIT_FIELD_TARGET, target.GetOwner().GetGUID());
}

void MovementState::UnbindOrientation()
{
    UnitBase::UnbindOrientation();
    GetOwner().SetUInt64Value(UNIT_FIELD_TARGET, 0);
}

void Scketches::ForceStop()
{
    MoveSplineInit(impl).MoveTo(impl.GetPosition3()).Launch();
}

void MovementState::UpdateState()
{
    uint32 now = getMSTime();
    int32 difftime = getMSTimeDiff(last_ms_time, now);
    last_ms_time = now;

    if (SplineEnabled())
    {
        MoveSpline::UpdateResult result;
        uint32 loops = 0;
        do
        {
            result = move_spline.updateState(difftime);
            SetPosition(move_spline.ComputePosition());

            if (++loops > 80)
            {
                log_console("MovementState::UpdateState: deadloop?");
                break;
            }

            switch (result & ~MoveSpline::Result_StopUpdate)
            {
            case MoveSpline::Result_NextSegment:
                log_console("MovementState::UpdateState: segment %d is on hold", move_spline.currentSplineSegment());
                // do something
                break;
            case MoveSpline::Result_Arrived:
                log_console("MovementState::UpdateState: spline done");
                // do something
                break;
           }
        }
        while(!(result & MoveSpline::Result_StopUpdate));

        if (move_spline.Finalized())
        {
            DisableSpline();
            ResetDirection();

            if (listener)
                listener->OnSplineDone();
        }
    }

    if (!SplineEnabled())
        updateRotation();
}

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
    flags.EnableFlying();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetWalk()
{
    flags.walkmode = true;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetSmooth()
{
    flags.EnableCatmullRom();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetCyclic()
{
    flags.cyclic = true;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFall()
{
    flags.EnableFalling();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetVelocity( float vel )
{
    velocity = vel;
    return *this;
}

void MoveSplineInit::Launch()
{
    if (target)
        state.BindOrientationTo(*target);
    else
        state.UnbindOrientation();

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

        MoveSplineUsed& spline = state.move_spline;
        spline.Initialize(*this);

        state.EnableSpline();
        state.SetForwardDirection();

        // shall MoveSpline initializer care about packet broadcasting?
        PacketBuilder::PathUpdate(state, MsgBroadcast(state));
    }
}

MoveSplineInit& MoveSplineInit::SetTrajectory( float max_height, float time_shift )
{
    time_perc = time_shift;
    parabolic_heigth = max_height;
    flags.EnableParabolic();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetKnockBack( float max_height, float time_shift )
{
    SetTrajectory(max_height, time_shift);
    flags.knockback = true;
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( MovementBase& t )
{
    facing.target = t.GetOwner().GetGUID();
    target = &t;
    flags.EnableFacingTarget();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( float o )
{
    facing.angle = G3D::wrap(o, 0.f, (float)G3D::twoPi());
    flags.EnableFacingAngle();
    return *this;
}

MoveSplineInit& MoveSplineInit::SetFacing( Vector3 const& spot )
{
    facing.spot.x = spot.x;
    facing.spot.y = spot.y;
    facing.spot.z = spot.z;
    flags.EnableFacingPoint();
    return *this;
}

MoveSplineInit::MoveSplineInit(MovementState& m) : state(m), target(NULL)
{
    state.UpdateState();
}

MoveSplineInit& MoveSplineInit::SetAnimation(AnimType anim, float anim_time)
{
    time_perc = anim_time;
    flags.EnableAnimation(anim);
    return *this;
}

void MsgBroadcast::operator ()(WorldPacket& data)
{
    m_owner.SendMessageToSet(&data, true);
}

}
