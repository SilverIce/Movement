#include "MoveUpdater.h"
#include "MovementBase.h"
#include "MaNGOS_API.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
        m_tick_time = MaNGOS_API::getMSTime();
        m_movers_count = 0;
    }

    void MoveUpdater::Update()
    {
        struct common_updater{
            inline void operator ()(UpdatableMovement* mov) { mov->UpdateState();}
        };

        m_tick_time = MaNGOS_API::getMSTime();
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
