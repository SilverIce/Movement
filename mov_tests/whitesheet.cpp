#pragma once

#include "simpleworld.h"
#include "movelistener.h"
#include "MoveUpdater.h"
#include "UnitMovement.h"

using namespace Movement;

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

struct WP_test : public TestArea, public IListener 
{
    WorldObject *fake;
    UnitMovementImpl& st;
    MoveUpdater updater;

    WP_test() : fake(NULL), st(*UnitMovementImpl::create(*fake))
    {
        st.SetListener(this);
        st.Initialize(Location(nodes2[0]),updater);

        move();
    }

    void move()
    {
        PointsArray path;
        for (int i = 0; i < CountOf(nodes2); ++i)
            path.push_back(nodes2[i]);

        MoveSplineInit init(st);
        init.MovebyPath(path);
        init.SetVelocity(15);
        init.SetCyclic();
        init.Launch();
    }

    void Update(const uint32 diff)
    {
        updater.Update();
    }

    virtual void OnEvent(const OnEventArgs& args) 
    {
        log_write("current position: %s", st.GetPosition3().toString().c_str());
        log_write("");
    }
};

void spline_sync_test();

void test()
{
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
