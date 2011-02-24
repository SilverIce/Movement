#include "MoveUpdater.h"
#include "Object.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
        m_tick_count = getMSTime();
        common_timer = 0;
        m_movers_count = 0;
    }

    void MoveUpdater::update()
    {
        struct final_updater
        {
            MovementBaseList& list;
            uint32 t_diff;

            final_updater(MovementBaseList& l, uint32 diff) : list(l), t_diff(diff) {}

            void operator ()(UpdaterLink& m)
            {
                MovementBase * mov = m.updatable;
                if (mov->delay >= 0)
                {
                    mov->delay -= t_diff;
                    if (mov->delay <= 0)
                        mov->UpdateState();
                }
            }
        };

        struct common_updater
        {
            MovementBaseList& list;
            uint32 t_diff;

            common_updater(MovementBaseList& l, uint32 diff) : list(l), t_diff(diff) {}

            void operator ()(UpdaterLink& m)
            {
                MovementBase * mov = m.updatable;
                if (mov->delay >= 0)
                    mov->delay -= t_diff;

                mov->UpdateState();
            }
        };

        uint32 now = getMSTime();
        uint32 diff = getMSTimeDiff(m_tick_count, now);
        m_tick_count = now;

        common_timer += diff;
        if (common_timer > Common_Update_Delay)
        {
            m_movers.Iterate(common_updater(m_movers,diff));
            common_timer = 0;
        }else
            m_movers.Iterate(final_updater(m_movers,diff));
    }
}
