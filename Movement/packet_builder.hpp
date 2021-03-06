#pragma once

namespace Movement
{
    enum MonsterMoveType
    {
        MonsterMoveNormal       = 0,
        MonsterMoveStop         = 1,
        MonsterMoveFacingSpot   = 2,
        MonsterMoveFacingTarget = 3,
        MonsterMoveFacingAngle  = 4
    };

    void PacketBuilder::WriteCommonMonsterMovePart(const UnitMovementImpl& mov, WorldPacket& data)
    {
        const MoveSpline& move_spline = mov.move_spline->moveSpline();
        MoveSplineFlag splineflags = move_spline.splineflags;

        if (mov.IsBoarded())
        {
            data.SetOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
            data << mov.Guid.WriteAsPacked();
            data << mov.GetTransport()->Guid.WriteAsPacked();
            data << int8(mov.m_unused.transport_seat);
        }
        else
        {
            data.SetOpcode(SMSG_MONSTER_MOVE);
            data << mov.Guid.WriteAsPacked();
        }

        data << uint8(0);       // boolean variable. toggles UnitMoveFlag::Unk4 flag
        data << mov.GetPosition3();
        data << move_spline.GetId();

        switch(splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
        default:
            data << uint8(MonsterMoveNormal);
            break;
        case MoveSplineFlag::Final_Target:
            data << uint8(MonsterMoveFacingTarget);
            data << move_spline.facing.target;
            break;
        case MoveSplineFlag::Final_Angle:
            data << uint8(MonsterMoveFacingAngle);
            data << move_spline.facing.angle;
            break;
        case MoveSplineFlag::Final_Point:
            data << uint8(MonsterMoveFacingSpot);
            data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        splineflags.enter_cycle = move_spline.isCyclic();
        data << uint32(splineflags & ~MoveSplineFlag::Mask_No_Monster_Move);

        if (splineflags.animation)
        {
            data << splineflags.getAnimationId();
            data << move_spline.effect_start_time;
        }

        data << move_spline.Duration();

        if (splineflags.parabolic)
        {
            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;
        }
    }

    void PacketBuilder::WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.getPointCount() - 3;
        const Vector3 * real_path = &spline.getPoint(1);

        data << last_idx;
        data << real_path[last_idx];   // destination
        if (last_idx > 1)
        {
            Vector3 middle = (real_path[0] + real_path[last_idx]) / 2.f;
            Vector3 offset;
            // first and last points already appended
            for(uint32 i = 1; i < last_idx; ++i)
            {
                offset = middle - real_path[i];
                data.appendPackXYZ(offset.x, offset.y, offset.z);
            }
        }
    }

    void PacketBuilder::WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << count;
        data.append<Vector3>(&spline.getPoint(2), count);
    }

    /*
    Mover GUID: 0x079A1 Unit 1E4B
    unk byte: 0
    Current Position: 6557.049 1156.459 385.3375
    sequence Id: 62345736
    MovementType: NORMAL
    Spline Flags: WALKMODE, FLYING, CYCLIC, ENTER_CYCLE
    Movement Time: 36937
    Points Count: 4
    Point 0: 6713.656 1158.745 357.573
    Point 1: 6627.521 1346.398 365.9617
    Point 2: 6532.312 1322.547 391.823
    Point 3: 6511.499 1175.65 393.3785

    Path Lenght 615.58657483662
    Speed  16.66585
    */

    void PacketBuilder::WriteCatmullRomCyclicPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 count = spline.getPointCount() - 3;
        data << uint32(count + 1);
        data << spline.getPoint(1); // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.getPoint(1), count);
    }

    void PacketBuilder::SplinePathSend(const UnitMovementImpl& mov)
    {
        mov_assert(mov.SplineEnabled());

        WorldPacket data(MSG_NULL_ACTION, 64);
        WriteCommonMonsterMovePart(mov, data);

        const MoveSpline& move_spline = mov.move_spline->moveSpline();
        const Spline<int32>& spline = move_spline.spline;
        MoveSplineFlag splineflags = move_spline.splineflags;
        if (splineflags & MoveSplineFlag::Mask_CatmullRom)
        {
            if (splineflags.cyclic)
                WriteCatmullRomCyclicPath(spline, data);
            else
                WriteCatmullRomPath(spline, data);
        }
        else
            WriteLinearPath(spline, data);

        Imports.BroadcastMessage(&mov.Owner, data);
    }

    void PacketBuilder::FullUpdate(const UnitMovementImpl& mov, ByteBuffer& data)
    {
        ClientMoveState state(mov.ClientState());
        WriteClientStatus(state,data);

        data.append<float>(&mov.m_float_values[SpeedWalk], Speed_End);

        if (state.moveFlags.spline_enabled)
        {
            const MoveSpline& move_spline = mov.move_spline->moveSpline();
            MoveSplineFlag splineFlags = move_spline.splineflags;

            data << splineFlags.raw;

            if (splineFlags.final_angle)
            {
                data << move_spline.facing.angle;
            }
            else if (splineFlags.final_target)
            {
                data << move_spline.facing.target;
            }
            else if(splineFlags.final_point)
            {
                data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            }

            data << move_spline.timePassed();
            data << move_spline.Duration();
            data << move_spline.GetId();

            data << float(1.f);//splineInfo.duration_mod;
            data << float(1.f);//splineInfo.duration_mod_next;

            data << move_spline.vertical_acceleration;
            data << move_spline.effect_start_time;

            uint32 nodes = move_spline.getPath().size();
            data << nodes;
            data.append<Vector3>(&move_spline.getPath()[0], nodes);
            data << uint8(move_spline.spline.mode());
            data << (move_spline.isCyclic() ? Vector3::zero() : move_spline.FinalDestination());
        }
    }

    void PacketBuilder::ReadClientStatus(ClientMoveState& mov, ByteBuffer& data)
    {
        data >> mov.moveFlags;
        data >> mov.ms_time;
        data >> mov.world_position;

        if (mov.moveFlags.ontransport)
        {
            mov.transport_guid = data.readPackGUID();
            data >> mov.transport_position;
            data >> mov.transport_time;
            data >> mov.transport_seat;

            if (mov.moveFlags.interp_move)
                data >> mov.transport_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying | UnitMoveFlag::Allow_Pitching))
        {
            data >> mov.pitchAngle;
        }

        data >> mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data >> mov.jump_verticalVelocity;
            data >> mov.jump_directionX;
            data >> mov.jump_directionY;
            data >> mov.jump_horizontalVelocity;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data >> mov.spline_elevation;
        }
    }

    void PacketBuilder::WriteClientStatus(const ClientMoveState& mov, ByteBuffer& data)
    {
        data << mov.moveFlags;
        data << mov.ms_time;
        data << mov.world_position;

        if (mov.moveFlags.ontransport)
        {
            data.appendPackGUID(mov.transport_guid);
            data << mov.transport_position;
            data << mov.transport_time;
            data << mov.transport_seat;

            if (mov.moveFlags.interp_move)
                data << mov.transport_time2;
        }

        if (mov.moveFlags & (UnitMoveFlag::Swimming | UnitMoveFlag::Flying | UnitMoveFlag::Allow_Pitching))
        {
            data << mov.pitchAngle;
        }

        data << mov.fallTime;

        if (mov.moveFlags.falling)
        {
            data << mov.jump_verticalVelocity;
            data << mov.jump_directionX;
            data << mov.jump_directionY;
            data << mov.jump_horizontalVelocity;
        }

        if (mov.moveFlags.spline_elevation)
        {
            data << mov.spline_elevation;
        }
    }

    void PacketBuilder::SplineSyncSend(const UnitMovementImpl& mov)
    {
        mov_assert(mov.SplineEnabled());
        const MoveSpline& move_spline = mov.move_spline->moveSpline();

        WorldPacket data(SMSG_FLIGHT_SPLINE_SYNC, 13);
        data << (float)(move_spline.timePassed() / (float)move_spline.Duration());
        data << mov.Guid.WriteAsPacked();
        Imports.BroadcastMessage(&mov.Owner, data);
    }

    void PacketBuilder::Send_HeartBeat(const UnitMovementImpl& mov)
    {
        WorldPacket data(MSG_MOVE_HEARTBEAT, 64);
        data << mov.Guid.WriteAsPacked();
        WriteClientStatus(mov.ClientState(), data);
        Imports.BroadcastMessage(&mov.Owner, data);
    }
}
