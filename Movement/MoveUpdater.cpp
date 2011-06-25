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
        struct common_updater{
            inline void operator ()(UpdatableMovement* mov) { mov->UpdateState();}
        };

        m_tick_time = getMSTime();
        m_movers.Iterate(common_updater());
    }

    void MoveUpdater::CleanReferences()
    {
        struct ref_cleaner{
            MoveUpdater * updater;
            ref_cleaner(MoveUpdater * u) : updater(u) {}
            inline void operator()(UpdatableMovement * mov) { mov->Dereference(updater);}
        };
        m_movers.Iterate(ref_cleaner(this));
        mov_assert(m_movers.empty());
    }
}
