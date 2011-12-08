#include "MovementBase.h"
#include "Imports.h"

namespace Movement{

    // for debugging:
    // there were problems with NaN coords in past
    inline bool _finiteV(const Vector3& v)
    {
        return _finite(v.x) && _finite(v.y) && _finite(v.z);
    }

    void MovementBase::SetGlobalPosition(const Location& loc)
    {
        assert_state(loc.isFinite());
        world_position = loc;
        Imports.UpdateMapPosition(&Owner, world_position);
    }

    MO_Transport::MO_Transport(WorldObjectType owner) : MovementBase(owner), m_transport(*this)
    {
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
            m_transport_link.List().delink(m_transport_link);

        m_transport_link.Value = TransportLink(&transport.Owner, this);
        transport._link_transportable(m_transport_link);

        m_local_position = local_position;
    }

    void Transportable::_unboard()
    {
        if (IsBoarded())
        {
            m_transport_link.List().delink(m_transport_link);
            m_transport_link.Value = TransportLink();

            m_local_position = Location();
        }
    }

    void Transportable::BoardOn(Transport& m, const Location& local_position, int8 seatId)
    {
        _board(m, local_position);
    }

    void Transportable::Unboard()
    {
        _unboard();
    }
}
