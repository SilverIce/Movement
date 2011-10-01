
/**
  file:         MoveUpdater.h
  author:       Silverice
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "LinkedList.h"
#include "TaskScheduler.h"
#include "typedefs_p.h"

namespace Movement
{
    using Tasks::TaskTarget;
    using Tasks::TaskExecutor_Args;
    using Tasks::CallBack;

    class UpdatableMovement;
    class MoveUpdater : public Tasks::TaskExecutor
    {
    public:

        explicit MoveUpdater();
        ~MoveUpdater() { CleanReferences();}
        void CleanReferences();

        void Register(LinkedListElement<UpdatableMovement*>& m)
        {
            if (!m.linked())
            {
                m_movers.link(m);
                ++m_movers_count;
            }
        }

        void Unregister(LinkedListElement<UpdatableMovement*>& m)
        {
            if (m.linked())
            {
                m_movers.delink(m);
                --m_movers_count;
            }
        }

        void Update();

        MSTime TickTime() const { return m_tick_time;}
        uint32 MoversCount() const { return m_movers_count;}
        uint32 NewMoveSplineId() { return movespline_counter.NewId();}

    private:

        LinkedList<UpdatableMovement*> m_movers;
        UInt32Counter movespline_counter;
        MSTime m_tick_time;
        uint32 m_movers_count;
    };

    extern MoveUpdater sMoveUpdater;
}
