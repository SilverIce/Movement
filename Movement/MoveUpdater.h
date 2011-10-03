
/**
  file:         MoveUpdater.h
  author:       Silverice
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs_p.h"
#include "TaskScheduler.h"

namespace Movement
{
    class MoveUpdater : public Tasks::TaskExecutor
    {
    public:

        explicit MoveUpdater();
        ~MoveUpdater();

        void Update();

        MSTime TickTime() const { return m_tick_time;}
        uint32 NewMoveSplineId() { return movespline_counter.NewId();}

    private:

        UInt32Counter movespline_counter;
        MSTime m_tick_time;
    };

    extern MoveUpdater sMoveUpdater;
}
