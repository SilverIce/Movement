
#include <sstream>

#include "UnitMovement.h"
#include "Client.h"

#include "MoveSpline.h"
#include "WorldPacket.h"
#include "MoveUpdater.h"
#include "Object.h"
#include "opcodes.h"
#include "MaNGOS_API.h"
#include "MoveListener.h"
#include "UnitMoveFlags.h"

#include "ClientMoveStatus.h"
#include "packet_builder.h"
#include "MoveSplineInit.h"
#include "UnitMovementImpl.h"
#include "ClientImpl.h"
#include "MovementMessage.h"

#include "MoveSplineInit.hpp"
#include "packet_builder.hpp"
#include "ClientImpl.hpp"
#include "UnitMovementImpl.hpp"
#include "UnitMovement.Requests.hpp"

namespace Movement
{
    UnitMovement* UnitMovement::create(WorldObject& owner)
    {
        char * data = (char*)operator new(sizeof(UnitMovement) + sizeof(UnitMovementImpl));
        UnitMovementImpl* impl = new(data + sizeof(UnitMovement))UnitMovementImpl(owner);
        return new(data)UnitMovement(*impl);
    }

    UnitMovement::~UnitMovement()
    {
        m.~UnitMovementImpl();
    }

    void UnitMovement::CleanReferences()
    {
        m.CleanReferences();
    }

    const Location& UnitMovement::GetPosition() const
    {
        return m.GetPosition();
    }

    bool UnitMovement::IsWalking() const
    {
        return m.moveFlags.walk_mode;
    }

    bool UnitMovement::IsMoving() const
    {
        return m.moveFlags & UnitMoveFlag::Mask_Moving;
    }

    bool UnitMovement::IsTurning() const
    {
        return m.moveFlags & (UnitMoveFlag::Turn_Left | UnitMoveFlag::Turn_Right);
    }

    bool UnitMovement::IsFlying() const
    {
        return m.moveFlags & (UnitMoveFlag::Flying | UnitMoveFlag::GravityDisabled);
    }

    bool UnitMovement::IsFalling() const
    {
        return m.moveFlags & (UnitMoveFlag::Falling);
    }

    bool UnitMovement::IsFallingFar() const
    {
        return m.moveFlags & (UnitMoveFlag::Fallingfar);
    }

    float UnitMovement::GetCollisionHeight() const
    {
        return m.GetParameter(Parameter_CollisionHeight);
    }

    float UnitMovement::GetSpeed(SpeedType type) const
    {
        return m.GetParameter((FloatParameter)(0 + type));
    }

    float UnitMovement::GetCurrentSpeed() const
    {
        return m.GetParameter(Parameter_SpeedCurrent);
    }

    void UnitMovement::UnboardAll()
    {
    }

    std::string UnitMovement::ToString() const
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

    Vector3 UnitMovement::direction() const
    {
        return m.direction();
    }

    void UnitMovement::Unboard()
    {
    }

    void UnitMovement::Board(UnitMovement& ps, const Location& local_pos, int8 seat)
    {
    }

    uint32 UnitMovement::MoveSplineId() const
    {
        return m.MoveSplineId();
    }

    const Vector3& UnitMovement::MoveSplineDest() const
    {
        return m.MoveSplineDest();
    }

    int32 UnitMovement::MoveSplineTimeElapsed() const
    {
        return m.MoveSplineTimeElapsed();
    }

    bool UnitMovement::HasMode(MoveMode mode) const
    {
        return m.HasMode(mode);
    }

    void UnitMovement::Teleport(const Location& loc)
    {
        m.Teleport(loc);
    }

    void UnitMovement::SetCollisionHeight(float value)
    {
        m.SetCollisionHeight(value);
    }

    void UnitMovement::ApplySwimMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeSwim, apply);
    }

    void UnitMovement::ApplyWalkMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeWalk, apply);
    }

    void UnitMovement::ApplyWaterWalkMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeWaterwalk, apply);
    }

    void UnitMovement::ApplySlowFallMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeSlowfall, apply);
    }

    void UnitMovement::ApplyFlyMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeFly, apply);
    }

    void UnitMovement::ApplyHoverMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeHover, apply);
    }

    void UnitMovement::ApplyRootMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeRoot, apply);
    }

    void UnitMovement::ApplyCanFlyMode( bool apply )
    {
        m.ApplyMoveMode(MoveModeCanFly, apply);
    }

    bool UnitMovement::IsBoarded() const
    {
        return m.IsBoarded();
    }

    void UnitMovement::WriteCreate(ByteBuffer& buf) const
    {
        PacketBuilder::FullUpdate(Impl(), buf);
    }

    void UnitMovement::SetListener(class IListener * l)
    {
        m.SetListener(l);
    }

    void UnitMovement::DisableGravity(bool apply)
    {
        m.ApplyMoveMode(MoveModeGravityDisabled, apply);
    }

    void UnitMovement::ApplyCanSwimFlyTransitionMode(bool apply)
    {
        m.ApplyMoveMode(MoveModeCanSwimFlyTransition, apply);
    }

    void UnitMovement::SetSpeed(SpeedType type, float speed)
    {
        m.SetSpeed(type, speed);
    }

    void UnitMovement::Initialize(const Location& position, MoveUpdater& updater)
    {
        m.Initialize(position, updater);
    }
}
