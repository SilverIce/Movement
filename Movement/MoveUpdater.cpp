#include "MoveUpdater.h"
#include "Imports.h"

namespace Movement
{
    MoveUpdater::MoveUpdater()
    {
    }

    void MoveUpdater::Update()
    {
        m_lastUpdate = Imports::getMSTime();
        Tasks::TaskExecutor::Update(m_lastUpdate);
    }

    MoveUpdater::~MoveUpdater()
    {
    }
}
