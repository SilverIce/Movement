
#include "UnitMovement.h"
#include "Client.h"
#include "MoveSplineInit.h"
#include "MoveListener.h"

#include "framework/Component.h"
#include "Imports.h"
#include "TaskScheduler.h"
#include "framework/DelayInit.h"
#include "framework/typedefs_p.h"

#include <sstream>
#include <list>
#include <hash_map>
#include <typeinfo>

//#include "Object.h"
#include "WorldPacket.h"
#include "opcodes.h"

#include "MoveSpline.h"
#include "MoveEnv.h"
#include "MovementBase.h"
#include "MovementCommandMgr.h"

#include "UnitMoveFlags.h"
#include "ClientMoveStatus.h"
#include "packet_builder.h"
#include "MovementMessage.h"
#include "UnitMovementImpl.h"
#include "UpdatableSpline.h"
#include "ClientImpl.h"
#include "UnitMovement.Passenger.h"
#include "UnitMovement.Vehicle.h"

#include "UnitMovement.Effects.hpp"
#include "MoveSplineInit.hpp"
#include "ClientImpl.hpp"
#include "UpdatableSpline.hpp"
#include "UnitMovementImpl.hpp"
#include "packet_builder.hpp"
#include "FallFormulas.hpp"

#include "UnitMovement.Tests.hpp"
#include "UnitMovement.Commands.hpp"

namespace Movement
{
    struct UnitMovementStruct
    {
        UnitMovement pubface;

        UnitMovementImpl unit;
        MoveSplineUpdatable monsterController;

        UnitMovementStruct(WorldObject& owner, uint64 ownerGuid, Tasks::ITaskExecutor& updater) : pubface(unit)
        {
            unit.ComponentInit(&unit);
            unit.ComponentAttach(&pubface);

            unit.Guid.SetRawValue(ownerGuid);
            unit.Owner = &owner;
            unit.Init(updater, &pubface);
            monsterController.Init(unit);

        }

        ~UnitMovementStruct()
        {
            Unit_Passenger::dealloc( unit.getAspect<Unit_Passenger>() );
            monsterController.CleanReferences();
            unit.CleanReferences();
        }
    };

    UnitMovement* UnitMovement::create(WorldObject& owner, uint64 ownerGuid, Tasks::ITaskExecutor& updater)
    {
        UnitMovementStruct* impl = new UnitMovementStruct(owner, ownerGuid, updater);
        return &impl->pubface;
    }

    void UnitMovement::dealloc()
    {
        delete ((UnitMovementStruct*)this);
    }

    void UnitMovement::CleanReferences()
    {
        m.CleanReferences();
    }

    void UnitMovement::SetPosition(const Location& position)
    {
        m.RelativeLocation(position);
    }

    Location UnitMovement::GetPosition()
    {
        return m.GetGlobalLocation();
    }

    const Vector3& UnitMovement::GetPosition3()
    {
        return m.GlobalPosition();
    }

    bool UnitMovement::IsWalking()
    {
        return m.IsWalking();
    }

    bool UnitMovement::IsMoving()
    {
        return m.IsMoving();
    }

    bool UnitMovement::IsTurning()
    {
        return m.IsTurning();
    }

    bool UnitMovement::IsFlying()
    {
        return m.IsFlying();
    }

    bool UnitMovement::IsFalling()
    {
        return m.IsFalling();
    }

    bool UnitMovement::IsFallingFar()
    {
        return m.IsFallingFar();
    }

    float UnitMovement::GetCollisionHeight()
    {
        return m.GetParameter(Parameter_CollisionHeight);
    }

    float UnitMovement::GetSpeed(SpeedType type)
    {
        return m.GetParameter((FloatParameter)(0 + type));
    }

    float UnitMovement::GetCurrentSpeed()
    {
        return m.GetCurrentSpeed();
    }

    std::string UnitMovement::ToString()
    {
        return m.ToString();
    }

    void UnitMovement::BindOrientationTo(UnitMovement& target)
    {
        m.getAspect<MoveSplineUpdatable>()->BindOrientationTo(target.Impl());
    }

    void UnitMovement::UnbindOrientation()
    {
        m.getAspect<MoveSplineUpdatable>()->UnbindOrientation();
    }

    Vector3 UnitMovement::direction()
    {
        return m.direction();
    }

    uint32 UnitMovement::MoveSplineId()
    {
        return m.getAspect<MoveSplineUpdatable>()->getCurrentMoveId();
    }

    bool UnitMovement::HasMode(MoveMode mode)
    {
        return m.HasMode(mode);
    }

    void UnitMovement::Teleport(const Location& loc)
    {
        TeleportEffect::Launch(&m, loc);
    }

    void UnitMovement::Knockback(float directionAngle, float horizontalVelocity, float verticalVelocity)
    {
        KnockbackEffect::Launch(m, directionAngle, horizontalVelocity, verticalVelocity);
    }

    void UnitMovement::SetCollisionHeight(float value)
    {
        FloatValueChangeEffect::Launch(&m, Parameter_CollisionHeight, value);
    }

    void UnitMovement::SetSpeed(SpeedType speed, float value)
    {
        FloatValueChangeEffect::Launch(&m, (FloatParameter)speed, value);
    }

    void UnitMovement::ApplyMoveMode(MoveMode mode, bool apply)
    {
        ModeChangeEffect::Launch(&m, mode, apply);
    }

    void UnitMovement::WriteCreate(ByteBuffer& buf)
    {
        PacketBuilder::FullUpdate(Impl(), buf);
    }

    void UnitMovement::SetListener(class IListener * listener)
    {
        m.getAspect<MoveSplineUpdatable>()->SetListener(listener);
    }

    Vehicle UnitMovement::asVehicle()
    {
        return Vehicle(m.getAspect<VehicleImpl>());
    }

    UnitPassenger UnitMovement::asPassenger()
    {
        return UnitPassenger(m.getAspect<Unit_Passenger>());
    }
}
