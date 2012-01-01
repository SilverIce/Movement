
#include "UnitMovement.h"
#include "Client.h"
#include "MoveSplineInit.h"

#include <sstream>
#include <list>
#include <hash_map>
#include <typeinfo>
#include "typedefs_p.h"

#include "Object.h"
#include "opcodes.h"
#include "WorldPacket.h"
#include "Imports.h"
#include "MoveSpline.h"
#include "MoveUpdater.h"
#include "MoveListener.h"
#include "MoveEnv.h"
#include "MovementBase.h"

#include "UnitMoveFlags.h"
#include "ClientMoveStatus.h"
#include "packet_builder.h"
#include "MovementMessage.h"
#include "UpdatableSpline.h"
#include "UnitMovementImpl.h"
#include "ClientImpl.h"

#include "MoveSplineInit.hpp"
#include "packet_builder.hpp"
#include "ClientImpl.hpp"
#include "UnitMovementImpl.hpp"
#include "UnitMovement.Effects.hpp"
#include "UpdatableSpline.hpp"

#include "UnitMovement.Tests.hpp"

namespace Movement
{
    UnitMovement* UnitMovement::create(WorldObjectType owner, uint64 ownerGuid, MoveUpdater& updater)
    {
        char * data = (char*)operator new(sizeof(UnitMovement) + sizeof(UnitMovementImpl));
        UnitMovementImpl* impl = new(data + sizeof(UnitMovement))UnitMovementImpl(owner, ownerGuid, updater);
        return new(data)UnitMovement(*impl);
    }

    void UnitMovement::dealloc()
    {
        m.~UnitMovementImpl();
        delete this;
    }

    void UnitMovement::CleanReferences()
    {
        m.CleanReferences();
    }

    void UnitMovement::SetPosition(const Location& position)
    {
        m.SetPosition(position);
    }

    Location UnitMovement::GetPosition()
    {
        return m.GetGlobalPosition();
    }

    const Vector3& UnitMovement::GetPosition3()
    {
        return m.GetPosition3();
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
        return m.GetParameter(Parameter_SpeedMoveSpline);
    }

    std::string UnitMovement::ToString()
    {
        return m.ToString();
    }

    void UnitMovement::BindOrientationTo(UnitMovement& target)
    {
        m.BindOrientationTo(target.Impl());
    }

    void UnitMovement::UnbindOrientation()
    {
        m.UnbindOrientation();
    }

    Vector3 UnitMovement::direction()
    {
        return m.direction();
    }

    uint32 UnitMovement::MoveSplineId()
    {
        return m.move_spline->getCurrentMoveId();
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
        m.move_spline->SetListener(listener);
    }
}
