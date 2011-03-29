#include "MovementBase.h"
#include "MoveUpdater.h"
#include "UnitMovement.h"
#include <float.h>

namespace Movement{

    void MovementBase::CleanReferences()
    {
        struct unbinder{
            inline void operator()(TargetLink& link) { link.targeter->UnbindOrientation();}
        };
        m_targeter_references.Iterate(unbinder());
        UnSheduleUpdate();
    }

    void MovementBase::SheduleUpdate(int32 delay_)
    {
        //mov_assert(updater_link.Value.updater);
        if (updater_link.Value.updater)
        {
            delay = delay_;
            updater_link.Value.updater->Register(updater_link);
        }
        else
            log_write("MovementBase::SheduleUpdate called, but updater is null");
    }

    void MovementBase::UnSheduleUpdate()
    {
        if (updater_link.linked())
            updater_link.Value.updater->Unregister(updater_link);
    }

    void GameobjectMovement::Board( Transport& m )
    {
        // TODO: add gameobject specific code
        _board(m);
    }

    void GameobjectMovement::UnBoard()
    {
        _unboard();
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
}
