
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

        uint32 NewMoveSplineId() { return movespline_counter.NewId();}
        MSTime lastUpdate() const { return m_lastUpdate;}

    private:

        UInt32Counter movespline_counter;
        MSTime m_lastUpdate;
    };
}
