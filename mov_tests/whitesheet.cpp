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
    Vector3(0,   0, 0),
};

inline float My_EvaluatePolynomial(double result, float *matrix, float t)
{
    for ( int degree = 1; degree <= (unsigned int)3; result = result * t + matrix[degree - 1] )
        ++degree;

    // if t is 0:  i.e.  result = matrix[2]
//     for ( int degree = 1; degree <= 3; result = matrix[degree - 1] )
//         ++degree;

    return result;
}

struct C44Matrix //struc ; (sizeof=0x40, standard type
{
    float a0;
    float a1;
    float a2;
    float a3;
    float b0;
    float b1;
    float b2;
    float b3;
    float c0;
    float c1;
    float c2;
    float c3;
    float d0;
    float d1;
    float d2;
    float d3;
};

void _Evaluate(const Vector3 *vertice, float t, C44Matrix *coeffs, Vector3 &position)
{
    int _4_cycles = 4;
    int i = 0;

    position = Vector3::zero();

    double c;
    while ( i < _4_cycles )
    {
        c = ((coeffs->a0 * t + coeffs->a1) * t + coeffs->a2) * t + coeffs->a3;

        position += c * (*vertice);

        coeffs = (C44Matrix *)((char *)coeffs + 16);// 16 -- sizeof(float x 4)

        ++i;
        ++vertice;
    }

    sLog.write("%f   %f", position.x, position.y);
//     sLog.write("%f   %f   %f", ST, position.x, position.y);
}

void test()
{
    C44Matrix s_catmullRomCoeffs;
    s_catmullRomCoeffs.a0 = -0.5;
    s_catmullRomCoeffs.a1 = 1;
    s_catmullRomCoeffs.a2 = -0.5;
    s_catmullRomCoeffs.a3 = 0;

    s_catmullRomCoeffs.b0 = 1.5;
    s_catmullRomCoeffs.b1 = -2.5;
    s_catmullRomCoeffs.b2 = 0;
    s_catmullRomCoeffs.b3 = 1;

    s_catmullRomCoeffs.c0 = -1.5;
    s_catmullRomCoeffs.c1 = 2;
    s_catmullRomCoeffs.c2 = 0.5;
    s_catmullRomCoeffs.c3 = 0;

    s_catmullRomCoeffs.d0 = 0.5;
    s_catmullRomCoeffs.d1 = -0.5;
    s_catmullRomCoeffs.d2 = 0;
    s_catmullRomCoeffs.d3 = 0;

    for (float i = 0; i < 1; i += 0.05f)
    {
        Vector3 c;
        _Evaluate(nodes, i, &s_catmullRomCoeffs, c);
    }


    return;
 
    SplinePure mover;
    mover.push_path(nodes, 4, SplineModeCatmullrom, true);

    GD3_spline catm;
    catm.cyclic = mover.cyclic;

    for (int i = 0; i < 4; ++i)
        catm.append( nodes[i]);
    catm.finalInterval = mover.finalInterval;

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
}

void World::InitWorld()
{
}
