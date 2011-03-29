#include "MovementBase.h"
#include "MoveUpdater.h"
#include "UnitMovement.h"
#include <float.h>

namespace Movement{

    void UpdatableMovement::ScheduleUpdate()
    {
        if (HasUpdater())
        {
            //delay = delay_;
            m_updater->Register(updater_link);
        }
        else
            log_write("UpdatableMovement::SheduleUpdate called, but updater is null");
    }

    void UpdatableMovement::UnScheduleUpdate()
    {
        if (HasUpdater())
            m_updater->Unregister(updater_link);
        else
            log_write("UpdatableMovement::UnSheduleUpdate called, but updater is null");
    }

    void MovementBase::CleanReferences()
    {
        struct unbinder{
            inline void operator()(TargetLink& link) { link.targeter->UnbindOrientation();}
        };
        m_targeter_references.Iterate(unbinder());
    }

    // for debugging:
    // there were problems with NaN coords in past
    inline bool _finiteV(const Vector3& v)
    {
        return _finite(v.x) && _finite(v.y) && _finite(v.z);
    }

    void MovementBase::SetPosition(const Location& v)
    {
        if (!_finiteV((Vector3&)v))
            log_write("MovementBase::SetPosition: NaN coord detected");
        else
            position = v;
    }

    void MovementBase::SetPosition(const Vector3& v)
    {
        if (!_finiteV(v))
            log_write("MovementBase::SetPosition: NaN coord detected");
        else
            (Vector3&)position = v;
    }

    void Transportable::_board(Transport& m)
    {
        _unboard();
        m_transport_link.Value = TransportLink(&m, this);
        m._link_transportable(m_transport_link);
    }

    void Transportable::_unboard()
    {
        if (m_transport_link.linked())
        {
            m_transport_link.Value = TransportLink();
            m_transport_link.delink();
        }
    }

    MO_Transport::MO_Transport(WorldObject& owner) : MovementBase(owner)
    {
        updatable.SetUpdateStrategy(this);
    }
}
