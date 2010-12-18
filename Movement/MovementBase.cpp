#include "MovementBase.h"

namespace Movement{

    void MovementBase::CleanReferences()
    {
        m_targeter_references.delink_all();
    }

    void UnitBase::Board( Transport& m )
    {
        m.m_passenger_references.link(m_transport_link, *this, m);
    }

    void UnitBase::UnBoard()
    {
        if (m_transport_link)
            m_transport_link.ref_from().m_passenger_references.delink(m_transport_link);
    }
}
