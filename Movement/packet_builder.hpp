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

    void PacketBuilder::WriteCommonMonsterMovePart(const UnitMovementImpl& mov, Packet& data)
    {
        const MoveSpline& move_spline = mov.as<MoveSplineUpdatable>().moveSpline();
        MoveSplineFlag splineflags = move_spline.splineflags;

        if (Unit_Passenger * passenger = mov.getAspect<Unit_Passenger>())
        {
            data.setOpcode(SMSG_MONSTER_MOVE_TRANSPORT);
            data << mov.Guid.WriteAsPacked();
            data << passenger->TransportGuid().WriteAsPacked();
            data.writeInt8(passenger->SeatId());
        }
        else
        {
            data.setOpcode(SMSG_MONSTER_MOVE);
            data << mov.Guid.WriteAsPacked();
        }

        data.writeInt8(0);
        data << mov.RelativePosition();
        data.writeUInt32(move_spline.GetId());

        switch(splineflags & MoveSplineFlag::Mask_Final_Facing)
        {
        default:
            data.writeUInt8(MonsterMoveNormal);
            break;
        case MoveSplineFlag::Final_Target:
            data.writeUInt8(MonsterMoveFacingTarget);
            data.writeUInt64(move_spline.facing.target);
            break;
        case MoveSplineFlag::Final_Angle:
            data.writeUInt8(MonsterMoveFacingAngle);
            data.writeSingle(move_spline.facing.angle);
            break;
        case MoveSplineFlag::Final_Point:
            data.writeUInt8(MonsterMoveFacingSpot);
            data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            break;
        }

        // add fake Enter_Cycle flag - needed for client-side cyclic movement (client will erase first spline vertex after first cycle done)
        splineflags.enter_cycle = move_spline.isCyclic();
        data.writeUInt32(splineflags & ~MoveSplineFlag::Mask_No_Monster_Move);

        if (splineflags.animation)
        {
            data.writeUInt8(splineflags.getAnimationId());
            data.writeInt32(move_spline.effect_start_time);
        }

        data.writeInt32(move_spline.timeTotal());

        if (splineflags.parabolic)
        {
            data.writeSingle(move_spline.vertical_acceleration);
            data.writeInt32(move_spline.effect_start_time);
        }
    }

    void PacketBuilder::WriteLinearPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        uint32 last_idx = spline.rawPoints().size() - 2 - 1;
        const Vector3 * real_path = &spline.rawPoints()[1];

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

                uint32 packed = 0;
                packed |= (int32(offset.x / 0.25f) & 0x7FF);
                packed |= (int32(offset.y / 0.25f) & 0x7FF) << 11;
                packed |= (int32(offset.z / 0.25f) & 0x3FF) << 22;
                data.writeUInt32(packed);
            }
        }
    }

    void PacketBuilder::WriteCatmullRomPath(const Spline<int32>& spline, ByteBuffer& data)
    {
        // first point(index 1) already appended, zero and last fake points are never appended at all
        uint32 count = spline.rawPoints().size() - 3;
        data.writeUInt32(count);
        data.append<Vector3>(&spline.rawPoints()[2], count);
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
        // first point(index 1) already appended, zero and last two fake points are never appended at all
        uint32 count = spline.rawPoints().size() - 3;
        data << uint32(count + 1);
        data << spline.rawPoints()[1]; // fake point, client will erase it from the spline after first cycle done
        data.append<Vector3>(&spline.rawPoints()[1], count);
    }

    void PacketBuilder::SplinePathSend(const UnitMovementImpl& mov)
    {
        Packet data(MSG_NULL_ACTION, 64);
        WriteCommonMonsterMovePart(mov, data);

        const MoveSpline& move_spline = mov.as<MoveSplineUpdatable>().moveSpline();
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

        Imports.BroadcastMessage(mov.Owner, data.toPacketData());
    }

    void PacketBuilder::FullUpdate(const UnitMovementImpl& mov, ByteBuffer& data)
    {
        const ClientMoveState& state = CreateMoveState(mov);

        WriteClientStatus(state,data);

        data.append<float>(&mov.m_float_values[SpeedWalk], Speed_End);

        if (state.moveFlags.spline_enabled)
        {
            const MoveSpline& move_spline = mov.as<MoveSplineUpdatable>().moveSpline();
            MoveSplineFlag splineFlags = move_spline.splineflags;

            data.writeInt32(splineFlags.raw);

            if (splineFlags.final_angle)
            {
                data.writeSingle(move_spline.facing.angle);
            }
            else if (splineFlags.final_target)
            {
                data.writeUInt64(move_spline.facing.target);
            }
            else if(splineFlags.final_point)
            {
                data << move_spline.facing.x << move_spline.facing.y << move_spline.facing.z;
            }

            data.writeInt32(move_spline.timePassed());
            data.writeInt32(move_spline.timeTotal());
            data.writeUInt32(move_spline.GetId());

            data.writeSingle(1.f);//splineInfo.duration_mod;
            data.writeSingle(1.f);//splineInfo.duration_mod_next;

            data.writeSingle(move_spline.vertical_acceleration);
            data.writeInt32(move_spline.effect_start_time);

            int32 pointCount = move_spline.spline.rawPoints().size();
            assert_state(pointCount >= 3);
            data.writeInt32(pointCount);
            data.append<Vector3>(move_spline.spline.rawPoints().constData(), pointCount);
            data.writeUInt8(move_spline.spline.mode());
            data << (move_spline.isCyclic() ? Vector3() : move_spline.FinalDestination());
        }
    }

    ClientMoveState PacketBuilder::CreateMoveState(const UnitMovementImpl& mov)
    {
        ClientMoveState state;
        static_cast<_ClientMoveState&>(state) = mov.m_unused;
        state.ms_time = Imports.getMSTime();
        state.globalPosition = mov.GetGlobalLocation();
        state.moveFlags = mov.moveFlags;
        state.pitchAngle = mov.PitchAngle();

        if (Unit_Passenger * psg = mov.getAspect<Unit_Passenger>())
        {
            state.moveFlags.ontransport = true;
            state.relativePosition = mov.RelativeLocation();
            state.transport_seat = psg->SeatId();
            state.transport_guid = psg->TransportGuid();
        }
        else
            state.moveFlags.ontransport = false;

        if (mov.SplineEnabled())
        {
            state.moveFlags = state.moveFlags & ~UnitMoveFlag::Mask_Directions | UnitMoveFlag::Forward;
            state.moveFlags.spline_enabled = true;
        }
        else
            state.moveFlags.spline_enabled = false;

        validateFLags(state.moveFlags);

        return state;
    }

    void PacketBuilder::ReadClientStatus(ClientMoveState& mov, ByteBuffer& data)
    {
        data >> mov.moveFlags;
        data >> mov.ms_time;
        data >> mov.globalPosition;

        if (mov.moveFlags.ontransport)
        {
            data >> mov.transport_guid.ReadAsPacked();
            data >> mov.relativePosition;
            data >> mov.transport_time;
            data >> mov.transport_seat;

            if (mov.moveFlags.interp_move)
                data >> mov.transport_time2;
        }

        if (mov.moveFlags & UnitMoveFlag::Mask_Pitching)
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
        data << mov.globalPosition;

        if (mov.moveFlags.ontransport)
        {
            data << mov.transport_guid.WriteAsPacked();
            data << mov.relativePosition;
            data << mov.transport_time;
            data << mov.transport_seat;

            if (mov.moveFlags.interp_move)
                data << mov.transport_time2;
        }

        if (mov.moveFlags & UnitMoveFlag::Mask_Pitching)
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
        const MoveSpline& move_spline = mov.as<MoveSplineUpdatable>().moveSpline();
        Packet data(SMSG_FLIGHT_SPLINE_SYNC, 13);
        data << (float)(move_spline.timePassed() / (float)move_spline.timeTotal());
        data << mov.Guid.WriteAsPacked();
        Imports.BroadcastMessage(mov.Owner, data.toPacketData());
    }

    void PacketBuilder::Send_HeartBeat(const UnitMovementImpl& mov)
    {
        Packet data(MSG_MOVE_HEARTBEAT, 64);
        data << mov.Guid.WriteAsPacked();
        WriteClientStatus(CreateMoveState(mov), data);
        Imports.BroadcastMessage(mov.Owner, data.toPacketData());
    }
}
