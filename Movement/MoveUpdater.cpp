#include "MoveUpdater.h"
#include "MaNGOS_API.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
        m_tick_time = MaNGOS_API::getMSTime();
    }

    void MoveUpdater::Update()
    {
        m_tick_time = MaNGOS_API::getMSTime();
        Tasks::TaskExecutor::Update(m_tick_time);
    }

    MoveUpdater::~MoveUpdater()
    {
        mov_assert(!Tasks::TaskExecutor::HasCallBacks());
    }
}
