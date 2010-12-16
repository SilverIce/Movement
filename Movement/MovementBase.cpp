#include "MovementBase.h"

namespace Movement{

    void MovementBase::CleanReferences()
    {
        m_targeter_references.delink_all();

        if (m_transport_link)
            ((TransportBase&)(*m_transport_link)).UnBoard(*this);
    }
}
