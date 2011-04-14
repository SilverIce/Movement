#pragma once

#include "simpleworld.h"
#include "UnitMovement.h"
#include "MoveSplineInit.h"
#include "movelistener.h"

#include "AllocationStatistic.h"
#include "MoveUpdater.h"
#include <sstream>
#include "MoveSpline.h"

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

    WorldObject *fake;
    UnitMovement st;
    MoveUpdater updater;

    WP_test() : st(*fake)
    {
        st.SetListener(this);
        st.Initialize(MovControlServer, Location(nodes2[0]),updater);
        st.SetUpdater(updater);

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
        updater.Update();
    }
};

void test_list();
void spline_sync_test();

void test()
{
    spline_sync_test();
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

#pragma region Spline sync test
struct SplineData 
{
    int32 duration;
    int32 move_time_passed;
    float duration_mod;
    float duration_mod_next;
};

void OnFlightSyncPacket(SplineData *_this, float path_passed_perc)
{
    SplineData *v2; // edx@1
    double v3; // st7@4
    SplineData *v4; // edx@8
    SplineData *v5; // esi@8
    double v6; // st7@8
    double v7; // st6@8

    v2 = _this;
    if ( v2 )
    {
        if ( v2->duration && (double)v2->duration * v2->duration_mod >= 0.009999999776482582 )
        {
            v3 = path_passed_perc - (double)(signed int)v2->move_time_passed / ((double)v2->duration * v2->duration_mod);
            if ( v3 > 0.5 )                           // 0.8
                v3 = v3 - 1.0;                          // 0.8-1 = -0.2
            if ( v3 < -0.5 )                          // -0.8
                v3 = v3 + 1.0;                          // -0.8+1 = 0.2
            v5 = _this;                      // v3 now in range [-0.5, 0.5]
            v7 = (double)(v5->duration - (unsigned int)(signed __int64)(v3 * (double)v5->duration)) / (double)v5->duration;
            v6 = 0.5;
            v5->duration_mod_next = v7;
            v4 = _this;
            if ( v4->duration_mod_next >= 0.5 && (v6 = v4->duration_mod_next, v6 >= 2.0) )
                v4->duration_mod_next = 2.0;
            else
                v4->duration_mod_next = v6;
        }
        else
        {
            v2->duration_mod_next = 1.0;
        }
    }
}

void spline_sync_test()
{
    SplineData dat = {90000, 33000, 1, 0};
    for (float t = 0.f; t <= 1.f; t += 0.1f)
    {
        dat.move_time_passed = dat.duration * t;
        OnFlightSyncPacket(&dat, t);
        if (!G3D::fuzzyEq(1, dat.duration_mod_next))
            log_write("spline_sync_test failed");
    }
}
#pragma endregion

void World::InitWorld()
{
    new WP_test();
}

void UpdateMapPosition(class WorldObject &,struct Movement::Location const &)
{
}
