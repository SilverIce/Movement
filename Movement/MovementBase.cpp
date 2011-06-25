#include "MovementBase.h"
#include "MoveUpdater.h"
#include "UnitMovement.h"
#include <float.h>
#include "MaNGOS_API.h"

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

    void MovementBase::SetGlobalPosition(const Location& loc)
    {
        world_position = loc;
        MaNGOS_API::UpdateMapPosition(&Owner, loc);
    }

    MovementBase::MovementBase(WorldObjectType owner) : Owner(owner), listener(NULL)
    {
    }

    MO_Transport::MO_Transport(WorldObjectType owner) : MovementBase(owner)
    {
        updatable.SetUpdateStrategy(this);
    }

    MO_Transport::~MO_Transport()
    {
        //delete impl;
    }

    void MO_Transport::UpdateState()
    {
        m_transport.Iterate(Transport::PassengerRelocator(GetGlobalPosition()));
    }

    void Transportable::board(const TransportData& transport, const Location& local_position)
    {
        if (transport.transport == m_transport)
        {
            log_write("Transportable::board: trying to board on same transport");
            return;
        }
        
        mov_assert(transport.transport != this);

        if (IsBoarded())
            m_transport_container->Remove(this);

        m_transport = transport.transport;
        m_transport_container = transport.container;
        m_transport_container->Add(this);

        m_local_position = local_position;
    }

    void Transportable::unboard()
    {
        if (IsBoarded())
        {
            m_transport_container->Remove(this);
            m_transport = NULL;
            m_transport_container = NULL;

            m_local_position = Location();
        }
    }
}
