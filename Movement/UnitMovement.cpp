
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
        }
    }

    if (!SplineEnabled())
        updateRotation();
}

void MsgBroadcast::operator ()(WorldPacket& data)
{
    m_owner.SendMessageToSet(&data, true);
}

}
