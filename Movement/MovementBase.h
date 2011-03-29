
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
    class UpdatableMovement;
    class Transportable;
    class UnitMovement;

    struct IUpdatable 
    {
        virtual void UpdateState() = 0;
    protected:
        ~IUpdatable(){}
    };

    class UpdatableMovement
    {
    private:
        LinkedListElement<UpdatableMovement*> updater_link;
        MoveUpdater * m_updater;
        IUpdatable * m_strategy;
    public:
        explicit UpdatableMovement() : m_updater(NULL), m_strategy(NULL)
        {
            updater_link.Value = this;
        }

        ~UpdatableMovement() { mov_assert(!IsUpdateScheduled());}

        void CleanReferences()
        {
            UnScheduleUpdate();
            m_updater = NULL;
        }

        void SetUpdateStrategy(IUpdatable * u) { m_strategy = u;}
        void ScheduleUpdate();
        void UnScheduleUpdate();
        bool IsUpdateScheduled() const { return updater_link.linked();}

        MoveUpdater& GetUpdater() const { mov_assert(m_updater);return *m_updater;}
        bool HasUpdater() const { return m_updater;}
        void SetUpdater(MoveUpdater& upd) { /*mov_assert(m_updater == NULL);*/m_updater = &upd;}
        void UpdateState() { m_strategy->UpdateState();}
    };

    /** Makes local transport position <--> global world position conversions */
    struct CoordTranslator
    {
        static Vector3 ToGlobal(const Vector3& coord_sys, const Vector3& local_coord)
        {
            return (coord_sys + local_coord);
        }

        static Vector3 ToLocal(const Vector3& coord_sys, const Vector3& local_coord)
        {
            return (coord_sys - local_coord);
        }

        static Vector3 ToGlobal(const Vector3& coord_sys, const Vector2& direction, const Vector3& local_coord)
        {
            Vector3 result(coord_sys);
            result.x += local_coord.x*direction.y - local_coord.y*direction.x;
            result.y += local_coord.x*direction.x + local_coord.y*direction.y;
            result.z += local_coord.z;
            return result;
        }

        static Vector3 ToLocal(const Vector3& coord_sys, const Vector2& direction, const Vector3& global_coord)
        {
            Vector3 result;
            Vector3 diff(coord_sys - global_coord);
            result.x = diff.x*direction.y + diff.y*direction.x;
            result.y = diff.y*direction.y - diff.x*direction.x;
            result.z = diff.z;
            return result;
        }

        static Location ToGlobal(const Location& coord_sys, const Vector2& direction, const Location& local_coord)
        {
            Location result = ToGlobal(static_cast<const Vector3&>(coord_sys),direction,static_cast<const Vector3&>(local_coord));
            // TODO: normalize orientation to be in range [0, 2pi)
            result.orientation = coord_sys.orientation + local_coord.orientation;
            return result;
        }

        static Location ToLocal(const Location& coord_sys, const Vector2& direction, const Location& global_coord)
        {
            Location result = ToLocal(static_cast<const Vector3&>(coord_sys),direction,static_cast<const Vector3&>(global_coord));
            // TODO: normalize orientation to be in range [0, 2pi)
            result.orientation = coord_sys.orientation - global_coord.orientation;
            return result;
        }

        static Location ToGlobal(const Location& coord_sys, const Location& local_coord)
        {
            Vector2 direction(cos(coord_sys.orientation), sin(coord_sys.orientation));
            return ToGlobal(coord_sys,direction,local_coord);
        }

        static Location ToLocal(const Location& coord_sys, const Location& global_coord)
        {
            Vector2 direction(cos(coord_sys.orientation), sin(coord_sys.orientation));
            return ToLocal(coord_sys,direction,global_coord);
        }
    };

    struct TargetLink
    {
        TargetLink() : target(0), targeter(0) {}

        TargetLink(MovementBase* target_, UnitMovement* targeter_)
            : target(target_), targeter(targeter_) {}

        MovementBase* target;
        UnitMovement* targeter;
    };

    class MovementBase
    {
    public:

        explicit MovementBase(WorldObject& owner) : m_owner(owner), listener(NULL)
        {
        }

        virtual ~MovementBase() { mov_assert(m_targeter_references.empty());}
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


        void _link_targeter(LinkedListElement<TargetLink>& t) { m_targeter_references.link(t);}

    protected:
        Location position;
        IListener * listener;
    private:

        LinkedList<TargetLink> m_targeter_references;
        WorldObject & m_owner;

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

        bool IsBoarded() const { return m_transport_link.linked();}
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


    /// concrete classes:

    class GameobjectMovement : public Transportable
    {
    public:

        explicit GameobjectMovement(WorldObject& owner) : Transportable(owner) {}

        virtual void Board(Transport& m);
        virtual void UnBoard();
    };

    class MO_Transport : public MovementBase, public IUpdatable
    {
    public:

        explicit MO_Transport(WorldObject& owner);
        virtual ~MO_Transport() {}

        virtual void CleanReferences()
        {
            m_transport.CleanReferences();
            MovementBase::CleanReferences();
            updatable.CleanReferences();
        }

        virtual void UpdateState() {}   // does nothing.. yet

        void UnBoardAll() { m_transport.UnBoardAll();}

        void ScheduleUpdate() { updatable.ScheduleUpdate();}
        void UnScheduleUpdate() { updatable.UnScheduleUpdate();}
        bool HasUpdater() const { return updatable.HasUpdater();}
        void SetUpdater(MoveUpdater& upd) { updatable.SetUpdater(upd);}

    private:

        Transport m_transport;
        UpdatableMovement updatable;
    };

}
