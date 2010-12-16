#pragma once

#include "typedefs.h"
#include "MoveListener.h"
#include "LinkedList.h"
#include "G3D/Vector3.h"
#include "G3D/Vector4.h"

class WorldObject;

namespace Movement
{
    class MovementBase
    {
    public:

        virtual ~MovementBase() {}

        virtual void CleanReferences();

        const Vector4& GetPosition() const { return position;}
        const Vector3& GetPosition3() const { return (Vector3&)position;}

        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

        explicit MovementBase(WorldObject& owner) : m_owner(owner) {}

    //protected:

        WorldObject & m_owner;
        IListener * listener;

        Vector4 position;
        Vector4 transport_offset;

        LinkedListElement<MovementBase> m_transport_link;
        LinkedList<MovementBase> m_targeter_references;
    };

    // parent class for unit- and gameobject-transports
    class TransportBase : public MovementBase
    {
    public:

        void Board(MovementBase& m)
        {
            m_passenger_references.link(m.m_transport_link, *this);
        }

        void UnBoard(MovementBase& m)
        {
            m_passenger_references.delink(m.m_transport_link);
        }

        void CleanReferences()
        {
            m_passenger_references.delink_all();
            MovementBase::CleanReferences();
        }

        explicit TransportBase(WorldObject& owner) : MovementBase(owner) {}

    protected:

        LinkedList<MovementBase> m_passenger_references;
    };

    // concrete class for unit's movement
    class UnitBase : public TransportBase
    {
    public:

        virtual void CleanReferences()
        {
            ResetTarget();
            TransportBase::CleanReferences();
        }

        void ResetTarget()
        {
            if (m_target_link)
                (*m_target_link).m_targeter_references.delink(m_target_link);
        }

        void SetTarget(MovementBase& m)
        {
            ResetTarget();
            // can i target self?
            m.m_targeter_references.link(m_target_link, m);
        }

    protected:

        explicit UnitBase(WorldObject& owner) : TransportBase(owner) {}

        LinkedListElement<MovementBase> m_target_link;
    };
}
