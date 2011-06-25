
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
#include <list>
#include <algorithm>

class WorldObject;

namespace Movement
{
    class MoveUpdater;
    class MovementBase;
    class UpdatableMovement;
    class Transportable;
    class IListener;
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

        void CleanReferences() { Dereference(m_updater);}

        void Dereference(MoveUpdater * updater)
        {
            mov_assert(updater == m_updater);
            UnScheduleUpdate();
            m_updater = NULL;
        }

        void SetUpdateStrategy(IUpdatable * u) { m_strategy = u;}
        void ScheduleUpdate();
        void UnScheduleUpdate();
        bool IsUpdateScheduled() const { return updater_link.linked();}
        MoveUpdater& GetUpdater() const { mov_assert(m_updater);return *m_updater;}
        bool HasUpdater() const { return m_updater != NULL;}
        void SetUpdater(MoveUpdater& upd) { mov_assert(m_updater == NULL);m_updater = &upd;}
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

    typedef WorldObject& WorldObjectType;

    class MovementBase
    {
    public:

        explicit MovementBase(WorldObjectType owner);

        virtual ~MovementBase() { mov_assert(m_targeter_references.empty());}
        virtual void CleanReferences();

        const Location& GetGlobalPosition() const { return world_position;}
        void SetGlobalPosition(const Location& loc);

        void SetListener(IListener * l) { listener = l;}
        void ResetLisener() { listener = NULL; }

        WorldObjectType Owner;

        void _link_targeter(LinkedListElement<TargetLink>& t) { m_targeter_references.link(t);}

    protected:

        IListener * listener;
        Location world_position;
    private:
        LinkedList<TargetLink> m_targeter_references;

        MovementBase(const MovementBase&);
        MovementBase& operator = (const MovementBase&);
    };

    class Transport;
    class Transportable;
    struct TransportData
    {
        TransportData() : transport(0), container(0) {}

        TransportData(MovementBase* transport_, Transport* _transport_container)
            : transport(transport_), container(_transport_container) {}

        MovementBase* transport;
        Transport* container;
    };

    class Transportable : public MovementBase
    {
    public:

        explicit Transportable(WorldObjectType owner) : MovementBase(owner), m_transport(NULL), m_transport_container(NULL)
        {
        }

        virtual ~Transportable() { mov_assert(!IsBoarded()); }

        virtual void CleanReferences()
        {
            Unboard();
            MovementBase::CleanReferences();
        }

        virtual void BoardOn(const TransportData& m, const Location& local_position) = 0;
        virtual void Unboard() = 0;

        bool IsBoarded() const { return m_transport != NULL;}
        const MovementBase* GetTransport() const { return m_transport;}

        const Location& GetLocalPosition() const { return m_local_position;}

    protected:

        void board(const TransportData& m, const Location& local_position);
        void unboard();

        Location m_local_position;
    private:
        MovementBase* m_transport;
        Transport* m_transport_container;
    };

    class Transport
    {
    public:

        typedef std::vector<Transportable*> ContainerType;

        struct PassengerRelocator
        {
            Location transport_pos;
            Vector2 dir;

            PassengerRelocator(const Location& transport_position)
                : transport_pos(transport_position),
                  dir(cos(transport_pos.orientation),sin(transport_pos.orientation))
            {
            }

            inline void operator()(Transportable* ps)
            {
                ps->SetGlobalPosition(CoordTranslator::ToGlobal(transport_pos, dir, ps->GetLocalPosition()));            
            }
        };

        struct Unboarder{
            inline void operator()(Transportable* ps) const { ps->Unboard(); }
        };

        template<class T> void Iterate(T func)
        {
            if (!m_passengers.empty())
            {
                ContainerType copy(m_passengers);
                std::for_each(copy.begin(),copy.end(),func);
            }
        }

        explicit Transport() {}
        ~Transport() { mov_assert(Empty());}

        bool Empty() const { return m_passengers.empty();}

        void Add(Transportable* ps) { m_passengers.push_back(ps);}
        void Remove(Transportable* ps) { m_passengers.erase(find(m_passengers.begin(),m_passengers.end(),ps));}

    private:
        ContainerType m_passengers;
    };

    class MO_Transport : public MovementBase, public IUpdatable
    {
    public:

        explicit MO_Transport(WorldObjectType owner);
        virtual ~MO_Transport();

        virtual void CleanReferences()
        {
            UnBoardAll();
            updatable.CleanReferences();
            MovementBase::CleanReferences();
        }

        virtual void UpdateState();

        void Board(Transportable& t, const Location& local_position)
        {
            t.BoardOn(TransportData(this,&m_transport), local_position);
        }

        void UnBoardAll() { m_transport.Iterate(Transport::Unboarder());}

        void SetUpdater(MoveUpdater& upd) { updatable.SetUpdater(upd);}

    private:

        Transport m_transport;
        UpdatableMovement updatable;
    };

    class StationaryMovObject : public Transportable
    {
    public:
        explicit StationaryMovObject(WorldObjectType owner) : Transportable(owner) {}

        void BoardOn(const TransportData& m, const Location& local_position)
        {
            Transportable::board(m, local_position);
        }

        virtual void Unboard()
        {
            Transportable::unboard();
        }
    };

}
