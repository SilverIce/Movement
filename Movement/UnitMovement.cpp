
#include "UnitMovement.h"
#include "WorldPacket.h"
#include "Object.h"
#include "moveupdater.h"
#include "MoveSpline.h"

namespace Movement{

SpeedType UnitMovement::SelectSpeedType( bool is_walking /*= false*/ ) const
{
    // g_moveFlags_mask - some global client's moveflag mask
    // TODO: get real value
    static uint32 g_moveFlags_mask = 0;

    //if ( !(g_moveFlags_mask & moveFlags) )
        //return 0.0f;

    if ( moveFlags.flying )
    {
        if ( moveFlags.backward /*&& speed_obj.flight >= speed_obj.flight_back*/ )
            return SpeedFlightBack;
        else
            return SpeedFlight;
    }
    else if ( moveFlags.swimming )
    {
        if ( moveFlags.backward /*&& speed_obj.swim >= speed_obj.swim_back*/ )
            return SpeedSwimBack;
        else
            return SpeedSwim;
    }
    else
    {
        if ( moveFlags.walk_mode || is_walking )
        {
            //if ( speed_obj.run > speed_obj.walk )
                return SpeedWalk;
        }
        else
        {
            if ( moveFlags.backward /*&& speed_obj.run >= speed_obj.run_back*/ )
                return SpeedRunBack;
        }
        return SpeedRun;
    }
}

static const uint32 Mode2Flag_table[]=
{
    {/*WALK_BEGAN,*/        UnitMoveFlag::Walk_Mode},
    {/*ROOT_BEGAN,*/        UnitMoveFlag::Root},
    {/*SWIM_BEGAN,*/        UnitMoveFlag::Swimming},
    {/*WATERWALK_MODE,*/    UnitMoveFlag::Waterwalking},
    {/*SLOW_FALL_BEGAN,*/   UnitMoveFlag::Safe_Fall},
    {/*HOVER_BEGAN,*/       UnitMoveFlag::Hover},
    {/*FLY_BEGAN, */        UnitMoveFlag::Flying},
    {                       UnitMoveFlag::Levitating},
    //{0},
};

void UnitMovement::ApplyMoveMode( MoveMode mode, bool apply )
{
    if (apply)
    {
        moveFlags |= Mode2Flag_table[mode];
        move_mode |= (1 << mode);
    }
    else
    {
        moveFlags &= ~Mode2Flag_table[mode];
        move_mode &= ~(1 << mode);
    }
}

static const float BaseSpeed[SpeedMaxCount] =
{
    2.5f,                                                   // SpeedWalk
    7.0f,                                                   // SpeedRun
    4.5f,                                                   // SpeedSwimBack
    4.722222f,                                              // SpeedSwim
    1.25f,                                                  // SpeedRunBack
    7.0f,                                                   // SpeedFlight
    4.5f,                                                   // SpeedFlightBack
    3.141594f,                                              // SpeedTurn
    3.141594f,                                              // SpeedPitch
};

UnitMovement::UnitMovement(WorldObject& owner) :
    Transportable(owner), move_spline(*new MoveSplineSegmented())
{
    updatable.SetUpdateStrategy(this);

    control_mode = MovControlServer;
    move_mode = 0;
    last_ms_time = 0;

    memcpy(&speed, BaseSpeed, sizeof BaseSpeed);
    speed_obj.current = BaseSpeed[SpeedRun];

    pitch = 0.f;
    // last fall time
    fallTime = 0;
    fallStartElevation = 0.f;
    // jumping
    j_velocity = j_sinAngle = j_cosAngle = j_xy_velocy = 0.f;
    u_unk1 = 0.f;
    speed_type = SpeedRun;
    dbg_flags = 0;
}

UnitMovement::~UnitMovement()
{
    delete &move_spline;
}

void UnitMovement::ReCalculateCurrentSpeed()
{
    speed_type = SelectSpeedType(false);
    speed_obj.current = speed[speed_type];
}

void UnitMovement::Initialize( MovControlType controller, const Location& pos)
{
    SetUpdater(sMoveUpdater);
    SetPosition(pos);

    control_mode = controller;
    last_ms_time = sMoveUpdater.TickCount();
}

void UnitMovement::ApplyState(const ClientMoveState& mov)
{
    moveFlags = mov.moveFlags;
    moveFlags2 = mov.moveFlags2;
    //last_ms_time = mov.ms_time;
    SetPosition(mov.position3);
    position.orientation = mov.orientation;

    m_transportInfo = mov.transport;
    pitch = mov.pitch;
    fallTime = mov.fallTime;
    j_velocity = mov.j_velocity;
    j_sinAngle = mov.j_sinAngle;
    j_cosAngle = mov.j_cosAngle;
    j_xy_velocy = mov.j_xy_velocy;
    u_unk1 = mov.u_unk1;
}

void UnitMovement::updateRotation(/*uint32 ms_time_diff*/)
{
    if (!IsOrientationBinded())
        return;

    const Vector3& t_pos = GetTarget()->GetPosition3();

    position.orientation = G3D::wrap(atan2(t_pos.y - position.y, t_pos.x - position.x), 0.f, (float)G3D::twoPi());

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

void UnitMovement::BindOrientationTo(MovementBase& target)
{
    UnbindOrientation();

    if (&target == this)
    {
        log_write("UnitMovement::BindOrientationTo: trying to target self, skipped");
        return;
    }

    // can i target self?
    m_target_link.Value = TargetLink(&target, this);
    target._link_targeter(m_target_link);
    Owner.SetUInt64Value(UNIT_FIELD_TARGET, target.Owner.GetGUID());
}

void UnitMovement::UnbindOrientation()
{
    m_target_link.Value = TargetLink();
    m_target_link.delink();
    Owner.SetUInt64Value(UNIT_FIELD_TARGET, 0);
}

void Scketches::ForceStop()
{
    MoveSplineInit(impl).MoveTo(impl.GetPosition3()).Launch();
}

void UnitMovement::SetSpeed(SpeedType type, float s)
{
    if (GetSpeed(type) != s)
    {
        speed[type] = s;
        PacketBuilder::SpeedUpdate(*this, type, MsgBroadcast(this));

        if (SplineEnabled() && type == getCurrentSpeedType())
        {
            if (G3D::fuzzyEq(s,0))
                Scketches(*this).ForceStop();
            else
            {
                // FIXME: currently there is no way to change speed of already moving server-side controlled unit (spline movement)
                // there is only one hacky way - launch new spline movement.. that's how blizz doing this
                Scketches(*this).ForceStop();
            }
        }
    }
}

void UnitMovement::UpdateState()
{
    uint32 now = sMoveUpdater.TickCount();
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
                log_console("UnitMovement::UpdateState: deadloop?");
                break;
            }

            switch (result & ~MoveSpline::Result_StopUpdate)
            {
            case MoveSpline::Result_NextSegment:
                log_console("UnitMovement::UpdateState: segment %d is on hold", move_spline.currentSplineSegment());
                // do something
                break;
            case MoveSpline::Result_Arrived:
                log_console("UnitMovement::UpdateState: spline done");
                // do something
                break;
           }
        }
        while(!(result & MoveSpline::Result_StopUpdate));

        if (move_spline.Finalized())
        {
            DisableSpline();
            ResetDirection();
            updatable.UnScheduleUpdate();
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
