
/**
  file:         MoveUpdater.h
  author:       Silverice
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "MovementBase.h"

namespace Movement
{
    class MoveUpdater
    {
    public:

        enum{
            Common_Update_Delay = 500
        };

        explicit MoveUpdater();

        void Register(MovementBaseLink& m)
        {
            if (!m.linked())
            {
                m_movers.link(m);
                ++m_movers_count;

                //log_console("Mover registered: name %s, movers count: %u", m.Value.updatable->GetOwner().GetName(), m_movers_count);
            }
        }

        void Unregister(MovementBaseLink& m)
        {
            if (m.linked())
            {
                m_movers.delink(m);
                --m_movers_count;

                //log_console("Mover unregistered: name %s, movers count: %u", m.Value.updatable->GetOwner().GetName(), m_movers_count);
            }
        }

        void update();

        uint32 TickCount() { return m_tick_count;}
        uint32 MoversCount() { return m_tick_count;}

    private:

        MovementBaseList m_movers;
        uint32 m_tick_count;
        uint32 common_timer;
        uint32 m_movers_count;
    };

    extern MoveUpdater sMoveUpdater;
}
