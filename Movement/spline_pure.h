#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "G3D/Vector3.h"

#include <assert.h>

enum SplineMode
{
    SplineModeLinear       = 0,
    SplineModeCatmullrom   = 1,
    SplineModeBezier3      = 2,
    SplineModeCount        = 3,
};

struct SplinePure
{
    std::vector<Vector3> points;
    std::vector<int> times;      // t0, t1, t2, t3 ... 
    std::vector<int> deltas;     // d0, d1, d2, d3 ...     t(N) = d(N-1) + d(N-2) + ... + d(0)

    bool cyclic;
    SplineMode mode;

    int computeIndexInBounds(int lastIdx, const int time_passed_delta) const;

    // assumes that 'time' can't be negative
    void evaluate(int time, Vector3 & c) const;

    // amount of time covered by spline in one period
    int duration() const { return times.back(); }

    int low_bound() const { return times.front();}
    int hight_bound() const { return times.back();}

    void SetfinalInterval(int t) { times[0] = t; }

    bool inBounds(int t) const { return times[0] <= t && t <= times.back(); }

    void computeIndex(int lastIndex, int& Index, int &X, float &u) const;

    void append(Vector3 control);

    void getControls(int i, int* t, Vector3* c, int N) const
    {
        for (int j = 0; j < N; ++j)
            getControl(i + j, t[j], c[j]);
    }

    void getControl(const int Idx, int& time, Vector3& c) const;

    void SetCyclic(bool cyclic_)
    {
        cyclic = cyclic_;
        times[0] = cyclic ? deltas[0] : 0;
    }

    void prepare()
    {
        int N = points.size();

        times.resize(N);
        deltas.resize(N);

        int i = 1;
        while(i < N)
        {
            deltas[i] = (points[i] - points[i-1]).length() / Movement::absolute_velocy * 1000.f;
            ++i;
        }
        deltas[0] = (points.back()-points.front()).length() / Movement::absolute_velocy * 1000.f;

        // updates times[0]
        SetCyclic(cyclic);

        i = 1;
        while(i < N)
        {
            times[i] = times[i-1] + deltas[i];
            ++i;
        }
    }

    void append(const Vector3 * controls, const int N)
    {
        points.resize(N);
        memcpy(&points[0],controls, sizeof(Vector3) * N);

        times.resize(N);
        deltas.resize(N);

        int i = 1;
        while(i < N)
        {
            deltas[i] = (points[i] - points[i-1]).length() / Movement::absolute_velocy * 1000.f;
            ++i;
        }
        deltas[0] = (points[N-1] - points[0]).length() / Movement::absolute_velocy * 1000.f;

        // updates times[0]
        SetCyclic(cyclic);

        i = 1;
        while(i < N)
        {
            times[i] = times[i-1] + deltas[i];
            ++i;
        }
    }

    SplinePure() : cyclic(false), mode(SplineModeLinear), times(2), deltas(2)
    {
    }
};


