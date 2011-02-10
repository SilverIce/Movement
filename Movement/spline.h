#pragma once

#include "typedefs.h"
#include "G3D/Vector3.h"
#include "G3D/Array.h"

#include <vector>

namespace Movement {


typedef std::vector<Vector3> PointsArray;

class Spline
{
public:

    typedef int time_type;
    typedef int index_type;

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeBezier3_Unused,
        ModesCount,
    };

    #pragma region fields
protected:

    PointsArray points;
    G3D::Array<float> lengths;

    index_type index_lo;
    index_type index_hi;
    index_type points_count;

    EvaluationMode m_mode;
    bool cyclic;

    // for internal use only!
    // calculates distance between [i; i+1] points,
    // assumes that index i is in bounds
    float SegLength(index_type i) const;

    void cacheLengths();

    index_type computeIndexInBounds(float length, float t) const;

protected:

    void EvaluateLinear(index_type, float, Vector3&) const;
    void EvaluateCatmullRom(index_type, float, Vector3&) const;
    void EvaluateBezier3(index_type, float, Vector3&) const;
    typedef void (Spline::*EvaluationMethtod)(index_type,float,Vector3&) const;
    static EvaluationMethtod evaluators[ModesCount];

    void EvaluateHermiteLinear(index_type, float, Vector3&) const;
    void EvaluateHermiteCatmullRom(index_type, float, Vector3&) const;
    void EvaluateHermiteBezier3(index_type, float, Vector3&) const;
    static EvaluationMethtod hermite_evaluators[ModesCount];

    float SegLengthLinear(index_type) const;
    float SegLengthCatmullRom(index_type) const;
    float SegLengthBezier3(index_type) const;
    typedef float (Spline::*SegLenghtMethtod)(index_type) const;
    static SegLenghtMethtod seglengths[ModesCount];

    void InitLinear(const Vector3*, const int, bool, int);
    void InitCatmullRom(const Vector3*, const int, bool, int);
    void InitBezier3(const Vector3*, const int, bool, int);
    typedef void (Spline::*InitMethtod)(const Vector3*, const int, bool, int);
    static InitMethtod initializers[ModesCount];

    enum{
        // could be modified, affects segment length evaluation precision
        // lesser value saves more performance in cost of lover precision
        // minimal value is 1
        // client's value is 20, blizzs use 2-3 steps to compute length
        STEPS_PER_SEGMENT = 2,
    };
    static_assert(STEPS_PER_SEGMENT > 0);
    #pragma endregion
public:

    explicit Spline();

    // 't' - percent of spline's length, assumes that t in range [0, 1]
    void evaluate_percent(float t, Vector3 & c) const;
    void evaluate_hermite(float t, Vector3& hermite) const;
    void evaluate_percent(index_type Idx, float u, Vector3& c) const
    {
        (this->*evaluators[m_mode])(Idx,u,c);
    }
    void evaluate_hermite(index_type Idx, float u, Vector3& hermite) const
    {
        (this->*hermite_evaluators[m_mode])(Idx,u,hermite);
    }

    void evaluate_percent_and_hermite(float t, Vector3 & c, Vector3& hermite) const;

    index_type computeIndexInBounds(float t) const;
    void computeIndex(float t, index_type& , float& u) const;

    void init_spline(const Vector3 * controls, const int N, EvaluationMode m);
    void init_cyclic_spline(const Vector3 * controls, const int N, EvaluationMode m, int cyclic_point);

    // returns length of the whole spline
    float length() const { return lengths[index_hi];}
    // returns length between given nodes
    float length(index_type first, index_type last) const { return lengths[last]-lengths[first];}
    float length(index_type Idx) const { return lengths[Idx];}
    float segment_length(index_type Idx) const { return lengths[Idx+1]-lengths[Idx];}

    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return m_mode;}
    bool isCyclic() const { return cyclic;}

    const PointsArray& getPoints() const { return points;}
    const Vector3& getPoint(index_type i) const { return points[i];}
    index_type pointsCount() const { return points_count;}

    void clear();

    std::string ToString() const;
};

}

