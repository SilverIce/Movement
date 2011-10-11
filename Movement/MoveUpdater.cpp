#include "MoveUpdater.h"
#include "MaNGOS_API.h"

namespace Movement
{
    MoveUpdater sMoveUpdater;

    MoveUpdater::MoveUpdater()
    {
    }

    void MoveUpdater::Update()
    {
        m_lastUpdate = MaNGOS_API::getMSTime();
        Tasks::TaskExecutor::Update(m_lastUpdate);
    }

    MoveUpdater::~MoveUpdater()
    {
        mov_assert(!Tasks::TaskExecutor::HasCallBacks());
    }
}
