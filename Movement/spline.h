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

typedef std::vector<Vector3> PointsArray;

class SplinePure
{
public:

    typedef int time_type;
    typedef int index_type;

protected:

    G3D::Array<Vector3> points;
    G3D::Array<float> lengths;

    index_type index_lo, index_hi;
    index_type points_count;
    bool cyclic;

    SplineMode m_mode;

    index_type computeIndexInBounds(float length, float t) const;

    // for internal use only!
    // calculates distance between [i; i+1] points,
    // assumes that index i is in bounds
    float SegLength(index_type i) const;

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

    void InitLinear(const Vector3*, const int, bool, int);
    void InitCatmullRom(const Vector3*, const int, bool, int);
    void InitBezier3(const Vector3*, const int, bool, int);
    typedef void (SplinePure::*InitPathPtr)(const Vector3*, const int, bool, int);
    static InitPathPtr initializers[SplineModeCount];

    enum{
        // could be modified, affects segment length evaluation precision
        // lesser value saves more performance in cost of lover precision
        // minimal value is 1
        // client's value is 20, blizzs use 2-3 steps to compute length
        STEPS_PER_SEGMENT = 2,
    };

    void cacheLengths();

public:

    explicit SplinePure();

    // 't' - percent of spline's length, assumes that t in range [0, 1]
    void evaluate_percent(float t, Vector3 & c) const;

    void init_spline(const Vector3 * controls, const int N, SplineMode m);
    void init_cyclic_spline(const Vector3 * controls, const int N, SplineMode m, int cyclic_point);

    // returns length of the whole spline
    float length() const { return lengths[index_hi];}
    // returns length between given nodes
    float length(index_type first, index_type last) const { return lengths[last]-lengths[first];}

    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    const Vector3* getCArray() const { return &points[index_lo];}
    index_type getCArraySize() const { return points_count;}

    bool empty() const { return index_lo == index_hi;}
    SplineMode mode() const { return m_mode;}
    bool isCyclic() const { return cyclic;}

    const G3D::Array<Vector3>& getPoints() const { return points;}
    const Vector3& getPoint(index_type i) const { return points[i];}
    index_type pointsCount() const { return points_count;}

    void clear();
    void erase(index_type i);

};

class SplineLive : public SplinePure
{
public:

    explicit SplineLive() : SplinePure(), m_current_node(0) {}

    void evaluate_percent(float t, Vector3 & c);

    void init_path(const Vector3 * controls, const int N, SplineMode m);
    void init_cyclic_path(const Vector3 * controls, const int N, SplineMode m, int cyclic_point);

    void reset_progress() { m_current_node = first(); }

    void clear()
    {
        SplinePure::clear();
        reset_progress();
    }

private:
    index_type computeIndexInBounds(float length, float t) const;

    index_type m_current_node;
};

}

