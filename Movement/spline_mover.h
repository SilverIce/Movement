#pragma once

#include "mov_constants.h"
#include "G3D/Vector3.h"
#include "G3D/Spline.h"

#include <vector>
#include <algorithm>
#include <functional>
#include "spline_pure.h"


struct less_equal_
    : public std::binary_function<unsigned int, unsigned int, bool>
{
    bool operator()(const unsigned int& _Left, const unsigned int& _Right) const
    {	// apply operator<= to operands
        return (_Left <= _Right);
    }
};

namespace Movement {


static void InterpolateLinear(uint32 Idx, uint32 time, const struct BaseMover & data, Vector3& v);

struct BaseMover
{
    std::vector<Vector3> points;
    std::vector<uint32> times;      // t0, t1, t2, t3 ... 
    //std::vector<uint32> deltas;     // d0, d1, d2, d3 ...     t(N) = d(N-1) + d(N-2) + ... + d(0)

    uint32 last_positionIdx;
    uint32 last_ms_time;

    uint32 time_passed;

    // amount of time covered by spline in one period
    uint32 duration;

    // 
    bool cyclic;
    SplineMode mode;

    uint32 size() const { return times.size(); }

    uint32 computeIndexInBounds(uint32 lastIdx, uint32 time_passed_delta) const
    { 
        uint32 N = times.size();
        while (lastIdx+1 < N && times[lastIdx+1] < time_passed_delta)
        {
            ++lastIdx;
        }
        return lastIdx;
    }

    void evaluate(uint32 curr_ms_time, float velocy, Vector3 & c)
    {
        // amount of time passed since last evaluate call
        uint32 t_passed = getMSTimeDiff(last_ms_time, curr_ms_time);
        last_ms_time = curr_ms_time;

        /** convert passed time to absolute passed time:
        *   if     absolute_velocy = velocy * alpha
        *   then   absolute_passed_time = t_passed / alpha;
            i.e.
            double alpha = absolute_velocy / velocy;
            t_passed = double(t_passed) / alpha;
        */
            t_passed = double(t_passed) * velocy / absolute_velocy;

        uint32 X = time_passed + t_passed;
        uint32 Index = 0;
        computeIndex(last_positionIdx, Index, X);

        // TODO: interpolate final coords with linear, catmullrom or bezier interpolation
        InterpolateLinear(Index, X, *this, c);

        last_positionIdx = Index;
        time_passed = X; 
    }

    BaseMover()
    {
        this->cyclic = false;

        this->last_ms_time = 0;
        this->last_positionIdx = 0;
        this->time_passed = 0;
        this->duration = 0;
    }

    void append(Vector3 c)
    {
        points.push_back(c);
    }

    void append2(Vector3 c)
    {
        points.push_back(c);
        uint32 N = points.size();
        if (N > 1)
        {
            uint32 time = (points[N-1] - points[N-2]).length() / absolute_velocy * 1000.f;
            times.push_back(time);
        }
        else
            times.push_back(0);

        duration = times.back();
    }

    void prepare(uint32 curr_ms_time)
    {
        last_ms_time = curr_ms_time;

        uint32 N = points.size();
        alwaysAssertM(N > 1,"");
        times.resize(N);
        uint32 i = 0, time = 0;
        while(true)
        {
            times[i] = time;
            if (++i >= N)
                break;
            time += (points[i] - points[i-1]).length() / absolute_velocy * 1000.f;
        }

        time_passed = 0;
        duration = times.back();
    }

    void computeIndex(uint32 lastIndex, uint32& Index, uint32 &X) const
    {
        uint32 N = times.size();

        if (X >= duration)
        {
            if (cyclic)
            {
                X = X % duration;
                // TODO: compute index instead of using null?
                Index = computeIndexInBounds(0, X);
            }
            else            // we are arrived
            {
                Index = N - 1;
                X = duration;
            }
        }
        else
        {
            // we are inside spline - this could be cyclic or not cyclic case
            Index = computeIndexInBounds(lastIndex, X);
        }

        alwaysAssertM(X <= duration,"X out of range");
        alwaysAssertM(Index < N,"Index out of range");
    }
};

void InterpolateLinear(uint32 Idx, uint32 time, const BaseMover & data, Vector3& v)
{
    if ( Idx+1 < data.times.size() )
    {
        alwaysAssertM(time >= data.times[Idx],"");

        double alpha = double(time - data.times[Idx]) / double(data.times[Idx+1] - data.times[Idx]);
        v = data.points[Idx] + (data.points[Idx+1] - data.points[Idx]) * alpha;
    }
    else
        v = data.points.back();
}


}
