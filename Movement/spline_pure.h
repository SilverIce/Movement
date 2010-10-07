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
    typedef int time_type;
    typedef int index_type;

    std::vector<Vector3> points;
    std::vector<time_type> times;      // t0, t1, t2, t3 ... 
    //std::vector<int> deltas;     // d0, d1, d2, d3 ...     t(N) = d(N-1) + d(N-2) + ... + d(0)

    int finalInterval;

    bool cyclic;

    index_type computeIndexInBounds(index_type lastIdx, const time_type time_passed_delta) const;

    time_type low_bound() const { return 0;}
    time_type hight_bound() const { return times.back();}

    //bool TimeInBounds(int t) const { return low_bound() < t && t < hight_bound(); }

    void computeIndex(index_type lastIndex, index_type& Index, time_type &X, float &u) const;

    void getControls(index_type i, time_type* t, Vector3* c, index_type N) const
    {
        for (index_type j = 0; j < N; ++j)
            getControl(i + j, t[j], c[j]);
    }

    void getControl(const index_type Idx, time_type& time, Vector3& c) const;

private:

    void InterpolateLinear(index_type, float, Vector3&) const;
    void InterpolateCatmullRom(index_type, float, Vector3&) const;
    void InterpolateBezier3(index_type, float, Vector3&) const;
    typedef void (SplinePure::*InterpolatorPtr)(index_type,float,Vector3&) const;
    static InterpolatorPtr interpolators[SplineModeCount];

    float SegLengthLinear(index_type) const;
    float SegLengthCatmullRom(index_type) const;
    float SegLengthBezier3(index_type) const;
    typedef float (SplinePure::*SegLenghtPtr)(index_type) const;
    static SegLenghtPtr seglengths[SplineModeCount];

public:

    SplineMode mode;

    // assumes that 'time' can't be negative
    void evaluate(time_type time, Vector3 & c) const;

    // amount of time covered by spline in one period
    time_type duration() const { return times.back() + finalInterval; }

    void push_path(const Vector3 * controls, const int N, SplineMode m, bool cyclic_)
    {
        cyclic = cyclic_;
        mode = m;
        points.resize(N);
        memcpy(&points[0],controls, sizeof(Vector3) * N);
        times.resize(N,0);

        //deltas.resize(N,0);

/*
        if (cyclic_)
            finalInterval = SegLength(N-1) / Movement::absolute_velocy * 1000.f;
        else
            finalInterval = 0;

        int i = 1;
        while(i < N)
        {
            times[i] = times[i-1] +
                SegLength(i-1) / Movement::absolute_velocy * 1000.f;
            ++i;
        }
*/
        if (cyclic_)
            finalInterval = (points[N-1] - points[0]).length() / Movement::absolute_velocy * 1000.f;
        else
            finalInterval = 0;

        int i = 1;
        while(i < N)
        {
            times[i] = times[i-1] +
                (points[i] - points[i-1]).length() / Movement::absolute_velocy * 1000.f;
            ++i;
        }

    }

    float length() const;

    float SegLength(int segment) const;

    SplinePure();
};


