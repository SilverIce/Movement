#include "MovementBase.h"
#include <float.h>

namespace Movement{

    void MovementBase::CleanReferences()
    {
        m_targeter_references.delink_all();
    }

    void UnitBase::Board( Transport& m )
    {
        m_transport_link.Value = TransportLink(&m, this); 
        m.m_passenger_references.link(m_transport_link);
    }

    void UnitBase::UnBoard()
    {
        if (m_transport_link)
        {
            m_transport_link.Value = TransportLink();
            m_transport_link.delink();
        }
    }

    void GameobjectMovement::Board( Transport& m )
    {
        m_transport_link.Value = TransportLink(&m, this); 
        m.m_passenger_references.link(m_transport_link);
    }

    void GameobjectMovement::UnBoard()
    {
        if (m_transport_link)
        {
            m_transport_link.delink();
            m_transport_link.Value = TransportLink();
        }
    }

    // for debugging:
    // there were problems with NaN coords in past
    inline bool _finiteV(const Vector3& v)
    {
        return _finite(v.x) && _finite(v.y) && _finite(v.z);
    }

    void MovementBase::SetPosition(const Vector4& v)
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
}
