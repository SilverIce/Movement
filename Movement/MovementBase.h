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

        // should be protected?
        void SetPosition(const Vector4& v);
        void SetPosition(const Vector3& v);

        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

        WorldObject& GetOwner() { return m_owner;}
        const WorldObject& GetOwner() const { return m_owner;}

    protected:

        friend class UnitBase;

        WorldObject & m_owner;
        IListener * listener;

        Vector4 position;

        LinkedList<MovementBase,MovementBase> m_targeter_references;
    };

    class Transport;

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

        Vector4 transport_offset;

        LinkedListElement<Transportable,Transport> m_transport_link;
    };

    class Transport
    {
        friend class MovementBase;
        friend class UnitBase;
        friend class GameobjectMovement;
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

        explicit Transport() {}

    protected:

        LinkedList<Transportable,Transport> m_passenger_references;
    };


    /// concretete classes:

    class GameobjectMovement : public Transportable
    {
    public:

        explicit GameobjectMovement(WorldObject& owner) : Transportable(owner) {}

        virtual void Board(Transport& m);
        virtual void UnBoard();
    };

    class MO_Transport : public MovementBase
    {
    public:

        explicit MO_Transport(WorldObject& owner) : MovementBase(owner) {}

        virtual void CleanReferences()
        {
            m_transport.CleanReferences();
            MovementBase::CleanReferences();
        }

        void UnBoardAll() { m_transport.UnBoardAll();}

    protected:
        Transport m_transport;
    };

    // class for unit's movement
    class UnitBase : public Transportable
    {
    public:

        virtual void CleanReferences()
        {
            UnbindOrientation();
            m_transport.CleanReferences();
            Transportable::CleanReferences();
        }

        void UnbindOrientation()
        {
            if (m_target_link)
                m_target_link.ref_from().m_targeter_references.delink(m_target_link);
        }

        void BindOrientationTo(MovementBase& m)
        {
            UnbindOrientation();
            // can i target self?
            m.m_targeter_references.link(m_target_link, *this, m);
        }

        bool IsOrientationBinded() const { return m_target_link; }

        virtual void Board(Transport& m);
        virtual void UnBoard();

    protected:

        explicit UnitBase(WorldObject& owner) : Transportable(owner) {}

        Transport m_transport;
        LinkedListElement<MovementBase,MovementBase> m_target_link;
    };
}
