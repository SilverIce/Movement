#include "MovementBase.h"
#include "Imports.h"

namespace Movement
{
    void MovementBase::SetGlobalPosition(const Location& loc)
    {
        assert_state(loc.isFinite());
        world_position = loc;
        Imports.OnPositionChanged(&Owner, world_position);
    }
}
