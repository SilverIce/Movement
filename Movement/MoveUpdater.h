
/**
  file:         MoveUpdater.h
  author:       Silverice
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "LinkedList.h"

namespace Movement
{
    class UpdatableMovement;
    class MoveUpdater
    {
    public:

        enum{
            Common_Update_Delay = 500
        };

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

        uint32 TickTime() const { return m_tick_time;}
        uint32 MoversCount() const { return m_movers_count;}
        uint32 NewMoveSplineId() { return movespline_counter.NewId();}

    private:

        LinkedList<UpdatableMovement*> m_movers;
        UInt32Counter movespline_counter;
        uint32 m_tick_time;
        uint32 common_timer;
        uint32 m_movers_count;
    };

    extern MoveUpdater sMoveUpdater;
}
