
/**
  file:         MovementBase.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "MoveListener.h"
#include "LinkedList.h"
#include "Location.h"

class WorldObject;

namespace Movement
{
    class MoveUpdater;
    class MovementBase;
    class Transportable;

    struct TargetLink
    {
        TargetLink() : target(0), targeter(0) {}

        TargetLink(MovementBase* target_, MovementBase* targeter_)
            : target(target_), targeter(targeter_) {}

        MovementBase* target;
        MovementBase* targeter;
    };

    struct UpdaterLink
    {
        UpdaterLink() : updatable(0), updater(0) {}

        UpdaterLink(MovementBase* updatable_, MoveUpdater* updater_)
            : updatable(updatable_), updater(updater_) {}

        MovementBase* updatable;
        MoveUpdater* updater;
    };
    typedef LinkedList<UpdaterLink> MovementBaseList;
    typedef LinkedListElement<UpdaterLink> MovementBaseLink;

    class Location : public Vector3
    {
    public:
        Location() : orientation(0) {}
        Location(float x, float y, float z, float o) : Vector3(x,y,z), orientation(0) {}

        float orientation;
    };

    class MovementBase
    {
    public:

        explicit MovementBase(WorldObject& owner) : m_owner(owner), listener(NULL), delay(0)
        {
            updater_link.Value = UpdaterLink(this, NULL);
        }

        virtual ~MovementBase() {}

        virtual void CleanReferences();

        const Location& GetPosition() const { return position;}
        const Vector3& GetPosition3() const { return position;}

        // should be protected?
        void SetPosition(const Location& v);
        void SetPosition(const Vector3& v);

        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

        WorldObject& GetOwner() { return m_owner;}
        const WorldObject& GetOwner() const { return m_owner;}

        /// Updates
        virtual void UpdateState() {}

        bool IsUpdateSheduled() const { return updater_link;}
        void SheduleUpdate(int32 delay);
        void UnSheduleUpdate();
        bool Initialized() const { return updater_link.Value.updater;}
        // should be protected?
        void SetUpdater(MoveUpdater& upd) { updater_link.Value.updater = &upd;}

        int32 delay;

        void _link_targeter(LinkedListElement<TargetLink>& t) { m_targeter_references.link(t);}

    protected:
        Location position;
        IListener * listener;
    private:

        LinkedList<TargetLink> m_targeter_references;
        WorldObject & m_owner;
        MovementBaseLink updater_link;

        MovementBase(const MovementBase&);
        MovementBase& operator = (const MovementBase&);
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

        bool IsBoarded() const { return m_transport_link;}
        Transport* GetTransport() { return m_transport_link.Value.transport;}
        const Transport* GetTransport() const { return m_transport_link.Value.transport;}

    protected:

        explicit Transportable(WorldObject& owner) : MovementBase(owner)  {}

        void _board(Transport& m);
        void _unboard();

        LinkedListElement<TransportLink> m_transport_link;
        Location transport_offset;
    };

    class Transport
    {
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

        bool HavePassengers() const { return !m_passenger_references.empty();}

        void _link_transportable(LinkedListElement<TransportLink>& t) { m_passenger_references.link(t);}

    private:

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
            m._link_targeter(m_target_link);
        }

        bool IsOrientationBinded() const { return m_target_link; }
        MovementBase* GetTarget() { return m_target_link.Value.target;}
        const MovementBase* GetTarget() const { return m_target_link.Value.target;}

        virtual void Board(Transport& m);
        virtual void UnBoard();

    protected:

        explicit UnitBase(WorldObject& owner) : Transportable(owner) {}

    private:
        Transport m_transport;
        LinkedListElement<TargetLink> m_target_link;
    };
}
