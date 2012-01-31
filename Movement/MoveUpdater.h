
/**
  file:         MoveUpdater.h
  author:       Silverice
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"
#include "Movement/TaskScheduler.h"

namespace Movement
{
    class MoveUpdater : public Tasks::TaskExecutor
    {
    public:

        EXPORT explicit MoveUpdater();
        EXPORT ~MoveUpdater();
        EXPORT void Update();

        uint32 NewMoveSplineId() { return movespline_counter.NewId();}
        MSTime lastUpdate() const { return m_lastUpdate;}

    private:

        UInt32Counter movespline_counter;
        MSTime m_lastUpdate;
    };
}
