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
        if (!_finiteV(v))
            log_write("MovementBase::SetPosition: NaN coord detected");
        else
            *managed_position = v;
    }

    void MovementBase::SetPosition(const Vector3& v)
    {
        if (!_finiteV(v))
            log_write("MovementBase::SetPosition: NaN coord detected");
        else
            (Vector3&)(*managed_position) = v;
    }

    MO_Transport::MO_Transport(WorldObject& owner) : MovementBase(owner), m_transport(*this)
    {
        updatable.SetUpdateStrategy(this);
    }

    MO_Transport::~MO_Transport()
    {
        //delete impl;
    }

    void MO_Transport::UpdateState()
    {
        m_transport.UpdatePassengerPositions();
    }

    void Transportable::_board(Transport& transport, const Location& local_position)
    {
        mov_assert(&transport.Owner != this);

        // should i unboard first?
        //_unboard();
        if (IsBoarded())
            m_transport_link.delink();

        m_transport_link.Value = TransportLink(&transport.Owner, this);
        transport._link_transportable(m_transport_link);

        m_local_position = local_position;
        set_managed_position(m_local_position);
    }

    void Transportable::_unboard()
    {
        if (IsBoarded())
        {
            m_transport_link.delink();
            m_transport_link.Value = TransportLink();

            set_managed_position(world_position);
            m_local_position = Location();
        }
    }
}
