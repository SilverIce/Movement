#include "MoveUpdater.h"
#include "Imports.h"

namespace Movement
{
    MoveUpdater::MoveUpdater()
    {
    }

    void MoveUpdater::Update()
    {
        Tasks::TaskExecutor::Update(Imports.getMSTime());
    }

    MoveUpdater::~MoveUpdater()
    {
    }
}
