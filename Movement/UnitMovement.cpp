
// clean public headers, implemented in current (UnitMovement) subsystem
#include "UnitMovement.h"
#include "Client.h"
#include "MoveSplineInit.h"
#include "MoveListener.h"

// clean public headers, implemented not here
#include "framework/Component.h"
#include "Imports.h"
#include "TaskScheduler.h"
#include "framework/DelayInit.h"
#include "framework/typedefs_p.h"

// external library headers
#include <QtCore/QTextStream>
#include <QtCore/QHash>
#include <QtCore/QVector>
#include <typeinfo>

// private headers, that may be referenced by multiple subsystems
#include "Packet.h"
#include "opcodes.h"
#include "MoveEnv.h"
#include "MovementBase.h"
#include "MovementCommandMgr.h"

// private headers, that belong to "unitmovement" subsystem
#include "MoveSpline.h"
#include "UnitMoveFlags.h"
#include "ClientMoveStatus.h"
#include "packet_builder.h"
#include "MovementMessage.h"
#include "UnitMovementImpl.h"
#include "UpdatableSpline.h"
#include "ClientImpl.h"
#include "UnitMovement.Passenger.h"
#include "UnitMovement.Vehicle.h"

// private implementation files, that belong to "unitmovement" subsystem
#include "UnitMovement.Effects.hpp"
#include "MoveSplineInit.hpp"
#include "ClientImpl.hpp"
#include "UpdatableSpline.hpp"
#include "packet_builder.hpp"
#include "FallFormulas.hpp"

#include "UnitMovement.Tests.hpp"
#include "UnitMovement.Commands.hpp"

namespace Movement
{
    struct UnitMovementStruct
    {
        UnitMovementImpl unit;
        MoveSplineUpdatable monsterController;

        UnitMovementStruct(const UnitMovement::CreateInfo& info, UnitMovement * publicFace)
        {
            unit.ComponentInit(&unit);
            unit.Init(info);
            unit.ComponentAttach(publicFace);
            monsterController.Init(unit);
        }

        ~UnitMovementStruct()
        {
            /** Bad solution: UnitMovementStruct destructor have to care about dynamically allocated components, have to know about them..
                How to solve? use ref counted pointers? or add a new wheel, 
            */
            Unit_Passenger::dealloc( unit.getAspect<Unit_Passenger>() );
            delete unit.getAspect<VehicleImpl>();

            monsterController.CleanReferences();
            unit.CleanReferences();
        }
    };

    UnitMovement::UnitMovement(const CreateInfo& info)
        : m(nullptr)
        , dbg_flags(0)
    {
        m = new UnitMovementStruct(info, this);
    }

    UnitMovement::~UnitMovement()
    {
        delete m;
        m = nullptr;
    }

    UnitMovementImpl& UnitMovement::Impl()
    {
        return m->unit;
    }

    void UnitMovement::SetPosition(const Location& position)
    {
        m->unit.RelativeLocation(Vector4(position.x,position.y,position.z,position.orientation));
    }

    Vector4 UnitMovement::GetPosition()
    {
        return m->unit.GetGlobalLocation();
    }

    const Vector3& UnitMovement::GetPosition3()
    {
        return m->unit.GlobalPosition();
    }

    bool UnitMovement::IsWalking()
    {
        return m->unit.IsWalking();
    }

    bool UnitMovement::IsMoving()
    {
        return m->unit.IsMoving();
    }

    bool UnitMovement::IsTurning()
    {
        return m->unit.IsTurning();
    }

    bool UnitMovement::IsFlying()
    {
        return m->unit.IsFlying();
    }

    bool UnitMovement::IsFalling()
    {
        return m->unit.IsFalling();
    }

    bool UnitMovement::IsFallingFar()
    {
        return m->unit.IsFallingFar();
    }

    float UnitMovement::GetCollisionHeight()
    {
        return m->unit.GetParameter(Parameter_CollisionHeight);
    }

    float UnitMovement::GetSpeed(SpeedType type)
    {
        return m->unit.GetParameter((FloatParameter)(0 + type));
    }

    float UnitMovement::GetCurrentSpeed()
    {
        return m->unit.GetCurrentSpeed();
    }

    QString UnitMovement::ToString()
    {
        return m->unit.toStringAll();
    }

    void UnitMovement::BindOrientationTo(UnitMovement& target)
    {
        m->monsterController.BindOrientationTo(target.Impl());
    }

    void UnitMovement::UnbindOrientation()
    {
        m->monsterController.UnbindOrientation();
    }

    Vector3 UnitMovement::direction()
    {
        return m->unit.movingDirection();
    }

    uint32 UnitMovement::MoveSplineId()
    {
        return m->monsterController.getCurrentMoveId();
    }

    bool UnitMovement::HasMode(MoveMode mode)
    {
        return m->unit.HasMode(mode);
    }

    void UnitMovement::Teleport(const Location& loc)
    {
        TeleportEffect::Launch(m->unit, Vector4(loc.x,loc.y,loc.z,loc.orientation));
    }

    void UnitMovement::Knockback(float directionAngle, float horizontalVelocity, float verticalVelocity)
    {
        KnockbackEffect::Launch(m->unit, directionAngle, horizontalVelocity, verticalVelocity);
    }

    void UnitMovement::SetCollisionHeight(float value)
    {
        FloatValueChangeEffect::Launch(m->unit, Parameter_CollisionHeight, value);
    }

    void UnitMovement::SetSpeed(SpeedType speed, float value)
    {
        FloatValueChangeEffect::Launch(m->unit, (FloatParameter)speed, value);
    }

    void UnitMovement::ApplyMoveMode(MoveMode mode, bool apply)
    {
        ModeChangeEffect::Launch(m->unit, mode, apply);
    }

    QByteArray UnitMovement::WriteCreate()
    {
        ByteBuffer buf(128);
        PacketBuilder::FullUpdate(m->unit, buf);
        return QByteArray(buf.contents(), buf.size());
    }

    void UnitMovement::SetListener(class IListener * listener)
    {
        m->monsterController.SetListener(listener);
    }

    Vehicle UnitMovement::asVehicle()
    {
        return Vehicle(getAspect<VehicleImpl>());
    }

    UnitPassenger UnitMovement::asPassenger()
    {
        return UnitPassenger(getAspect<Unit_Passenger>());
    }
}
