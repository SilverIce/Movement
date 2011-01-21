#pragma once

#include "typedefs.h"
#include "MoveListener.h"
#include "LinkedList.h"
#include "G3D/Vector3.h"
#include "G3D/Vector4.h"

class WorldObject;

namespace Movement
{
    class MovementBase;

    struct TargetLink 
    {
        TargetLink() : target(0), targeter(0) {}

        TargetLink(MovementBase* target_, MovementBase* targeter_)
            : target(target_), targeter(targeter_) {}

        MovementBase* target;
        MovementBase* targeter;
    };

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

        LinkedList<TargetLink> m_targeter_references;
        Vector4 position;
        WorldObject & m_owner;
        IListener * listener;
    };

    class Transport;
    class Transportable;

    struct TransportLink 
    {
        TransportLink() : transport(0), transportable(0) {}

        TransportLink(Transport* transport_, Transportable* transportable_)
            : transport(transport_), transportable(transportable_) {}

        Transport* transport;
        Transportable* transportable;
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

        Transport* GetTransport() { return m_transport_link.Value.transport;}
        const Transport* GetTransport() const { return m_transport_link.Value.transport;}

    protected:

        explicit Transportable(WorldObject& owner) : MovementBase(owner)  {}

        LinkedListElement<TransportLink> m_transport_link;
        Vector4 transport_offset;
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
                inline void operator()(TransportLink& m) const { m.transportable->UnBoard(); }
            };
            m_passenger_references.Iterate(_unboard());
        }

        void CleanReferences()
        {
            UnBoardAll();
        }

        explicit Transport() {}

    protected:

        LinkedList<TransportLink> m_passenger_references;
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
                m_target_link.delink();
        }

        void BindOrientationTo(MovementBase& m)
        {
            UnbindOrientation();
            // can i target self?
            m_target_link.Value = TargetLink(&m, this);
            m.m_targeter_references.link(m_target_link);
        }

        bool IsOrientationBinded() const { return m_target_link; }
        MovementBase* Target() { return m_target_link.Value.target;}
        const MovementBase* Target() const { return m_target_link.Value.target;}

        virtual void Board(Transport& m);
        virtual void UnBoard();

    protected:

        explicit UnitBase(WorldObject& owner) : Transportable(owner) {}

        Transport m_transport;
        LinkedListElement<TargetLink> m_target_link;
    };
}
