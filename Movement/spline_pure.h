#pragma once

#include "typedefs.h"
#include "client_constants.h"
#include "G3D/Vector3.h"

#include <assert.h>

enum SplineMode
{
    SplineModeLinear,
    //SplineMode_G3D_Catmullrom,
    SplineModeCatmullrom,
    SplineModeBezier3,
    SplineModeCount,
};

struct SplinePure
{
    typedef int time_type;
    typedef int index_type;

    std::vector<Vector3> points;
    std::vector<time_type> times;

    float full_length;
    std::vector<float> lengths;

    index_type index_lo, index_hi;

    SplineMode mode;
    bool cyclic;

    index_type computeIndexInBounds(index_type lastIdx, const time_type time_passed_delta) const;

    time_type low_bound() const { return times[index_lo];}
    time_type hight_bound() const { return times[index_hi];}

    void computeIndex(index_type lastIndex, index_type& Index, time_type &X, float &u) const;

    // returns distance between [i; i+1] points
    // assumes that index i in range [1; N - 2]
    float SegLength(index_type i) const;

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

    enum{
        // could be modified, affects segment legth evaluation precision
        STEPS_PER_SEGMENT = 20,
    };
public:

    // assumes that 'time' can't be negative
    void evaluate(time_type time, Vector3 & c) const;

    // amount of time covered by spline in one period
    time_type duration() const { return hight_bound() - low_bound(); }

    void push_path(const Vector3 * controls, const int N, SplineMode m, bool cyclic_);

    // returns lenth of the spline
    float length() const { return full_length; }

    SplinePure();
};


