#pragma once

#include "typedefs.h"
#include "G3D/Vector3.h"
#include "G3D/Array.h"

#include <vector>

namespace Movement {

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

    typedef std::vector<Vector3> PointsArray;

    PointsArray points;
    std::vector<float> lengths;
    //G3D::Array<Vector3> points;
    //G3D::Array<float> lengths;

    index_type index_lo, index_hi;
    bool cyclic;

    SplineMode m_mode;

    index_type computeIndexInBounds(float length, float t) const;

    void computeIndex(index_type lastIndex, index_type& Index, time_type &X, float &u) const;

    // for internal use only!
    // calculates distance between [i; i+1] points,
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

    explicit SplinePure();

    // 't' - percent of spline's length, assumes that t in range [0, 1]
    void evaluate_percent(float t, Vector3 & c) const;


    void init_path(const Vector3 * controls, const int N, SplineMode m, bool cyclic_);

    // returns length of the whole spline
    float length() const { return lengths[index_hi];}

    SplineMode mode() const { return m_mode;}

};

}

