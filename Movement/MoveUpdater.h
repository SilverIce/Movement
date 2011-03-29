
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
        ~MoveUpdater() { CleanReferences(); mov_assert(m_movers.empty());}
        void CleanReferences();

        void Register(LinkedListElement<UpdatableMovement*>& m)
        {
            if (!m.linked())
            {
                m_movers.link(m);
                ++m_movers_count;

                //log_console("Mover registered: name %s, movers count: %u", m.Value.updatable->GetOwner().GetName(), m_movers_count);
            }
        }

        void Unregister(LinkedListElement<UpdatableMovement*>& m)
        {
            if (m.linked())
            {
                m_movers.delink(m);
                --m_movers_count;

                //log_console("Mover unregistered: name %s, movers count: %u", m.Value.updatable->GetOwner().GetName(), m_movers_count);
            }
        }

        void Update();

        uint32 TickCount() { return m_tick_count;}
        uint32 MoversCount() { return m_movers_count;}

    private:

        LinkedList<UpdatableMovement*> m_movers;
        uint32 m_tick_count;
        uint32 common_timer;
        uint32 m_movers_count;
    };

    extern MoveUpdater sMoveUpdater;
}
