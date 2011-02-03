#pragma once

#include "simpleworld.h"
#include "g3d_based.h"
#include "spline.h"
#include "UnitMovement.h"
#include "ByteBuffer.h"
#include "movelistener.h"

#include "AllocationStatistic.h"

static G3D::Matrix4 g3d_catmullrom_basis2(
    0.5f, 2.f, -2.f, 0.5f,
    -1.f, -3.f, 3.f, -0.5f,
    0.5f, 0.f, 0.f, 0.f,
    -0.f, 1.f, 0.f, 0.f);

/*
Update Flags: UPDATEFLAG_HIGHGUID, UPDATEFLAG_LIVING, UPDATEFLAG_HAS_POSITION
Movement Flags: FORWARD, LEVITATING, SPLINE_ENABLED
Unknown Flags: 0000
Timestamp: B9D33A89
Position: -4030.4 1140.276 99.04669 5.01662
Fall Time: 00000000
Speed0: 1.66667
Speed1: 16
Speed2: 4.5
Speed3: 4.722222
Speed4: 2.5
Speed5: 7
Speed6: 4.5
Speed7: 3.141593
Speed8: 3.141593
Spline Flags: WALKMODE, FLYING, CYCLIC
Spline CurrTime: 619
Spline FullTime: 16420
Spline Unk: 0000043C
Spline float1: 1
Spline float2: 1
Spline float3: 0
Spline uint1: 00000000
Spline Count: 0000000B
Splines_0: -4015.405 1186.572 107.8463
Splines_1: -4032.093 1150.835 100.8185
Splines_2: -4016.442 1117.503 95.84628
Splines_3: -3982.193 1100.35 95.62405
Splines_4: -3950.59 1116.853 99.12405
Splines_5: -3933.569 1150.704 103.263
Splines_6: -3949.776 1184.953 106.8185
Splines_7: -3981.939 1200.395 108.2629
Splines_8: -4015.405 1186.572 107.8463
Splines_9: -4032.093 1150.835 100.8185
Splines_10: -4016.442 1117.503 95.84628
Spline byte3: 01
Spline End Point: 0 0 0
High GUID: 08FCFF88
*/

// 259.37573
// time 16420
// vel 15.78
Vector3 nodes[] =
{
    //Vector3(	-4015.405,	1186.572,	107.8463	),
    Vector3(	-4032.093,	1150.835,	100.8185	),
    Vector3(	-4016.442,	1117.503,	95.84628	),
    Vector3(	-3982.193,	1100.35,    95.62405	),
    Vector3(	-3950.59,	1116.853,	99.12405	),
    Vector3(	-3933.569,	1150.704,	103.263	    ),
    Vector3(	-3949.776,	1184.953,	106.8185	),
    Vector3(	-3981.939,	1200.395,	108.2629	),
    Vector3(	-4015.405,	1186.572,	107.8463	),
    //Vector3(	-4032.093,	1150.835,	100.8185	),
    //Vector3(	-4016.442,	1117.503,	95.84628	),
};

// 253.20219
// vel 13.45

// Spline CurrTime: 1764
// Spline FullTime: 18818
Vector3 nodes2[] =
{
    //Vector3(-3982.866,	950.2649,	58.96975),
    /*1*/    Vector3(-4000.046,	985.8019,	61.02531),
    Vector3(-3981.982,	1017.846,	58.96975),
    Vector3(-3949.962,	1033.053,	56.85864),
    Vector3(-3918.825,	1014.746,	58.33086),
    Vector3(-3900.323,	984.7424,	60.60864),
    Vector3(-3918.999,	953.8466,	58.96975),
    Vector3(-3950.793,	934.2088,	58.96975),

    Vector3(-3982.866,	950.2649,	58.96975),
    /*9*/    //Vector3(-4000.046,	985.8019,	61.02531),
    //Vector3(-3981.982,	1017.846,	58.96975),
};



struct Coords3
{
    float x, y, z;
};


Coords3 tt[] =
{
    {1, 0, 0},
    {10, 0, 0},
    {20, 0, 0},
    {40, 0, 0},
};

struct WP_test : public TestArea, public IListener 
{
    void OnEvent(int eventId, int data)
    {
        log_write("OnEvent: eventId %d, point %d", eventId, data);
        log_write("current position: %s", st.GetPosition3().toString().c_str());
        log_write("");
    }
    void OnSplineDone()
    {
        log_write("SplineDone");
        log_write("");

        //move();
    }

    WorldObject * fake;
    MovementState st;

    WP_test() : st(fake)
    {
        st.SetListener(this);
        st.SetPosition(nodes2[0]);

        move();
    }

    void move()
    {
        PointsArray path;
        for (int i = 0; i < CountOf(nodes2); ++i)
            path.push_back(nodes2[i]);

        MoveSplineInit(st).MovebyPath(path).SetVelocity(15).SetCyclic().Launch();
    }

    void Update(const uint32 diff)
    {
        st.UpdateState();
    }
};

void test_list();

void test()
{
    sWorld.Run();
}

void test_list()
{
    LinkedListElement<int> elements[10];
    LinkedList<int> list;
    for (int i = 0; i < CountOf(elements); ++i)
    {
        elements[i].Value = i;
        list.link(elements[i]);
    }

    struct _counter
    {
        int count;
        _counter() : count(0) {}
        void operator()(int& i) {++count;}
    };

    _counter cc;
    list.Visit(cc);
}

void World::InitWorld()
{
    new WP_test();
}
