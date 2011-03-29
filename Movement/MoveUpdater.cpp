#include "MoveUpdater.h"
#include "MovementBase.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
        m_tick_count = getMSTime();
        common_timer = 0;
        m_movers_count = 0;
    }

    void MoveUpdater::Update()
    {
        /*struct final_updater
        {
            MovementBaseList& list;
            uint32 t_diff;

            final_updater(MovementBaseList& l, uint32 diff) : list(l), t_diff(diff) {}

            void operator ()(UpdaterLink& m)
            {
                UpdatableMovement * mov = m.updatable;
                if (mov->delay >= 0)
                {
                    mov->delay -= t_diff;
                    if (mov->delay <= 0)
                        mov->UpdateState();
                }
            }
        };*/

        struct common_updater
        {
            uint32 t_diff;

            common_updater(uint32 diff) : t_diff(diff) {}

            void operator ()(UpdatableMovement* mov)
            {
                //if (mov->delay >= 0)
                    //mov->delay -= t_diff;
                mov->UpdateState();
            }
        };

        uint32 now = getMSTime();
        uint32 diff = getMSTimeDiff(m_tick_count, now);
        m_tick_count = now;

        //common_timer += diff;
        //if (common_timer > Common_Update_Delay)
        //{
            m_movers.Iterate(common_updater(diff));
            //common_timer = 0;
        //}else
            //m_movers.Iterate(final_updater(m_movers,diff));
    }

    void MoveUpdater::CleanReferences()
    {
        struct ref_cleaner{
            inline void operator()(UpdatableMovement * mov) { mov->CleanReferences();}
        };
        m_movers.Iterate(ref_cleaner());
        mov_assert(m_movers.empty());
    }
}
