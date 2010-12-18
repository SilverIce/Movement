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

        explicit MovementBase(WorldObject& owner) : m_owner(owner), listener(NULL) {}
        virtual ~MovementBase() {}

        virtual void CleanReferences();

        const Vector4& GetPosition() const { return position;}
        const Vector3& GetPosition3() const { return (Vector3&)position;}

        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

    protected:

        friend class UnitBase;

        WorldObject & m_owner;
        IListener * listener;

        Vector4 position;

        LinkedList<MovementBase,MovementBase> m_targeter_references;
    };

    class Transportable : public MovementBase
    {
    public:

        virtual ~Transportable() {}

        virtual void Board(Transport& m) = 0;
        virtual void UnBoard() = 0;

        virtual void CleanReferences()
        {
            UnBoard();
            MovementBase::CleanReferences();
        }

    protected:

        explicit Transportable(WorldObject& owner) : MovementBase(owner)  {}

        LinkedListElement<Transportable,Transport> m_transport_link;
        Vector4 transport_offset;
    };

    class Transport
    {
        friend class MovementBase;
        friend class UnitBase;
    public:

        void UnBoardAll()
        {
            struct _unboard{
                inline void operator()(Transportable& m) const { m.UnBoard(); }
            };
            m_passenger_references.Iterate(_unboard());
        }

        void CleanReferences()
        {
            UnBoardAll();
        }

    protected:

        explicit Transport() {}

        LinkedList<Transportable,Transport> m_passenger_references;
    };


    /// concretete classes:

    class GameobjectMovement : public Transportable
    {
    public:

        explicit GameobjectMovement(WorldObject& owner) : Transportable(owner) {}

        virtual void Board(Transport& m) {}
        virtual void UnBoard() {}
    };

    class MO_Transport : public MovementBase, public Transport
    {
    public:

        explicit MO_Transport(WorldObject& owner) : MovementBase(owner) {}

    };

    // class for unit's movement
    class UnitBase : public Transportable, public Transport
    {
    public:

        virtual void CleanReferences()
        {
            ResetTarget();
            Transport::CleanReferences();
            Transportable::CleanReferences();
        }

        void ResetTarget()
        {
            if (m_target_link)
                m_target_link.ref_from().m_targeter_references.delink(m_target_link);
        }

        void SetTarget(MovementBase& m)
        {
            ResetTarget();
            // can i target self?
            m.m_targeter_references.link(m_target_link, *this, m);
        }

        virtual void Board(Transport& m);
        virtual void UnBoard();

    protected:

        explicit UnitBase(WorldObject& owner) : Transportable(owner) {}

        LinkedListElement<MovementBase,MovementBase> m_target_link;
    };
}
