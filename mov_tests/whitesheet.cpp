#pragma once

#include "movement.h"
#include "simpleworld.h"
#include "PathMover.h"
#include "worldpacket_fake.h"
#include "spline_mover.h"


struct Unit : public TestArea
{
    MovementState st;
    LinearMover mover;

    Unit() : mover(&st) {}

    void Update(const uint32 diff)
    {
        if (mover.Update_short((int32)diff))        // if true -- arrived then 
            stop();
    }

    void InitTest()
    {

    }
};

void test()
{
    SplineMover mover;
    mover.cyclic = true;
 
    for (float x = -20, times = 0; x < 20;)
    {
        float y = x*x;

        mover.append(times, Vector3(x,y,y));

        x += 4;
        times += 1;
    }

    Vector3 position = mover.UpdatePosition(0);
;
    position = mover.UpdatePosition(4);
;
    position = mover.UpdatePosition(4.4);
;
    position = mover.UpdatePosition(80);
;
    //new Unit;
    //sWorld.Run();
}

void World::InitWorld()
{
}

