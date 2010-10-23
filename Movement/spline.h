#pragma once

#include "typedefs.h"
#include "G3D/Vector3.h"
#include "G3D/Array.h"

#include <assert.h>

enum SplineMode
{
    SplineModeLinear,
    SplineModeCatmullrom,
    SplineModeBezier3_Unused,
    SplineModeCount,
};

class SplinePure
{
public:

    typedef int time_type;
    typedef int index_type;

    G3D::Array<Vector3> points;
    G3D::Array<time_type> times;
    G3D::Array<float> lengths;

    float full_length;

    index_type index_lo, index_hi;

    SplineMode mode;
    bool cyclic;

    index_type computeIndexInBounds(index_type lastIdx, const time_type time_passed_delta) const;
    index_type computeIndexInBounds(time_type time_passed_delta) const;

    time_type low_bound() const { return times[index_lo];}
    time_type hight_bound() const { return times[index_hi];}

    void computeIndex(index_type lastIndex, index_type& Index, time_type &X, float &u) const;

    // returns distance between [i; i+1] points
    // assumes that index i is in bounds
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

    void InitLinear(const Vector3*, const int);
    void InitCatmullRom(const Vector3*, const int);
    void InitBezier3(const Vector3*, const int);
    typedef void (SplinePure::*InitPathPtr)(const Vector3*, const int);
    static InitPathPtr initializers[SplineModeCount];

    enum{
        // could be modified, affects segment length evaluation precision
        STEPS_PER_SEGMENT = 20,
    };

    void cacheLengths();

public:

    // assumes that 'time' can't be negative
    void evaluate(const time_type time, Vector3 & c) const;
    void evaluate(time_type& time, Vector3 & c) const;

    // amount of time covered by spline in one period
    time_type duration() const { return hight_bound() - low_bound(); }

    void init_path(const Vector3 * controls, const int N, SplineMode m, bool cyclic_);

    // returns length of the whole spline
    float length() const { return full_length; }

    SplinePure();
};


