#pragma once

#include "movement.h"
#include "simpleworld.h"
#include "PathMover.h"
#include "worldpacket_fake.h"
#include "spline_mover.h"
#include "g3d_based.h"
#include "outLog.h"
#include "spline_pure.h"

struct Unit : public TestArea
{
    BaseMover mover;
    float speed;

    uint32 local_time;

    Unit()
    {
        mover.cyclic = false;
        for (float x = -20; x <= 20; x += 4)
        {
            float y = x*x;
            mover.append(Vector3(x,y,0));
        }
        local_time = 0;
        mover.prepare( local_time );

        speed = 7;
        sWorld.SLEEP_TIME = 50;
        sLog.write("Base mover log started, ms time %u, speed %f", local_time, speed);
    }

    void Update(const uint32 diff)
    {
        local_time += 3000;

        Vector3 vec;
        mover.evaluate(local_time, speed, vec);
        sLog.write("Spline evaluated at %u, diff %u, position is %s", local_time, 3000, vec.toString().c_str());
        if (mover.time_passed >= mover.duration)
        {
            // arrived, stop out test
            stop();
            sLog.write("Base mover arrived, end of logging");
        }
    }
};


static G3D::Matrix4 g3d_catmullrom_basis2(
    0.5f, 2.f, -2.f, 0.5f,
    -1.f, -3.f, 3.f, -0.5f,
    0.5f, 0.f, 0.f, 0.f,
    -0.f, 1.f, 0.f, 0.f);


Vector3 nodes[]=
{
    Vector3(0,     -0.5,   0),
    Vector3(2,     2,   0),
    Vector3(3,     -1.5, 0),
    Vector3(1.7,   0, 0),
};


void test()
{
 
    SplinePure mover;
    mover.push_path(nodes, 4, SplineModeCatmullrom, true);

    GD3_spline catm;
    catm.cyclic = mover.cyclic;

    for (int i = 0; i < 4; ++i)
        catm.append( nodes[i]);
    catm.finalInterval = mover.finalInterval;

    //mover.SetfinalInterval(250);
    //catm.finalInterval = 250;
//     for (int i = -2; i < 6;  )
//     {
//         Vector3 c1;
//         Vector3 c2;
//         float t1 = 0;
//         int t2 = 0;
// 
//         catm.getControl(i, t1, c1);
//         mover.getControl(i, t2, c2);
//         ++i;
//     }

    sLog.write("G3D spline:");
    float dur = catm.duration(), part = dur/20;
    for (float i = 0; i <= dur; i += part )
    {
        Vector3 c;
        catm.evaluate(i, c, g3d_catmullrom_basis2);
        sLog.write("%f   %f", c.x, c.y);
    }

    sLog.write("\nMine spline:");
    dur = mover.duration(), part = dur/20;
    for (float i = 0; i <= dur; i += part )
    {
        Vector3 v;
        mover.evaluate(i, v);
    }

    sLog.write("\nLengths:");
    for (int i = 0; i <= 6; ++i )
    {
        float l = mover.SegLength(i);
        sLog.write("%i   %f", i, l);
    }
}

void World::InitWorld()
{
}
