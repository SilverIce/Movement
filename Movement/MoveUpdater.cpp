#include "MoveUpdater.h"
#include "MovementBase.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
        m_tick_time = getMSTime();
        common_timer = 0;
        m_movers_count = 0;
    }

    void MoveUpdater::Update()
    {
        struct common_updater
        {
            uint32 t_diff;
            common_updater(uint32 diff) : t_diff(diff) {}
            void operator ()(UpdatableMovement* mov) { mov->UpdateState();}
        };

        uint32 now = getMSTime();
        uint32 diff = getMSTimeDiff(m_tick_time, now);
        m_tick_time = now;
        m_movers.Iterate(common_updater(diff));
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
