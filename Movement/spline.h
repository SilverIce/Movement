
/**
  file:         spline.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "typedefs.h"
#include "G3D/Vector3.h"

namespace Movement {

class SplineBase
{
public:
    typedef int index_type;
    typedef std::vector<Vector3> ControlArray;

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeBezier3_Unused,
    };

    #pragma region fields
protected:
    ControlArray points;

    index_type index_lo;
    index_type index_hi;
    index_type points_count;

    uint8 m_mode;
    bool cyclic;

    enum{
        // could be modified, affects segment length evaluation precision
        // lesser value saves more performance in cost of lover precision
        // minimal value is 1
        // client's value is 20, blizzs use 2-3 steps to compute length
        STEPS_PER_SEGMENT = 3,

        UninitializedMode = 3,
        ModesCount = UninitializedMode+1,
    };
    static_assert(STEPS_PER_SEGMENT > 0);

protected:
    void EvaluateLinear(index_type, float, Vector3&) const;
    void EvaluateCatmullRom(index_type, float, Vector3&) const;
    void EvaluateBezier3(index_type, float, Vector3&) const;
    typedef void (SplineBase::*EvaluationMethtod)(index_type,float,Vector3&) const;
    static EvaluationMethtod evaluators[ModesCount];

    void EvaluateHermiteLinear(index_type, float, Vector3&) const;
    void EvaluateHermiteCatmullRom(index_type, float, Vector3&) const;
    void EvaluateHermiteBezier3(index_type, float, Vector3&) const;
    static EvaluationMethtod hermite_evaluators[ModesCount];

    float SegLengthLinear(index_type) const;
    float SegLengthCatmullRom(index_type) const;
    float SegLengthBezier3(index_type) const;
    typedef float (SplineBase::*SegLenghtMethtod)(index_type) const;
    static SegLenghtMethtod seglengths[ModesCount];

    void InitLinear(const Vector3*, index_type, bool, index_type);
    void InitCatmullRom(const Vector3*, index_type, bool, index_type);
    void InitBezier3(const Vector3*, index_type, bool, index_type);
    typedef void (SplineBase::*InitMethtod)(const Vector3*, index_type, bool, index_type);
    static InitMethtod initializers[ModesCount];

    void UninitializedSpline() const { mov_assert(false);}

    #pragma endregion
public:

    explicit SplineBase() : m_mode(UninitializedMode), index_lo(0), index_hi(0), cyclic(false) {}

    /** Caclulates the position for given segment Idx, and percent of segment length t
        @param t - percent of segment length, assumes that t in range [0, 1]
        @param Idx - spline segment index, should be in range [first, last)
     */
    void evaluate_percent(index_type Idx, float u, Vector3& c) const {(this->*evaluators[m_mode])(Idx,u,c);}

    /** Caclulates Vector3(dx/dt, dy/dt, dz/dt) for index Idx, and percent of segment length t
        @param Idx - spline segment index, should be in range [first, last)
        @param t  - percent of spline segment length, assumes that t in range [0, 1]
     */
    void evaluate_hermite(index_type Idx, float u, Vector3& hermite) const {(this->*hermite_evaluators[m_mode])(Idx,u,hermite);}

    /**  Bounds for spline indexes. All indexes should be in range [first, last). */
    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return (EvaluationMode)m_mode;}
    bool isCyclic() const { return cyclic;}

    const ControlArray& getPoints() const { return points;}
    const Vector3& getPoint(index_type i) const { return points[i];}
    index_type pointsCount() const { return points_count;}

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void init_spline(const Vector3 * controls, index_type count, EvaluationMode m);
    void init_cyclic_spline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point);
    void clear();

    /** Calculates distance between [i; i+1] points, assumes that index i is in bounds. */
    float SegLength(index_type i) const { return (this->*seglengths[m_mode])(i);}

    std::string ToString() const;
};

template<typename length_type>
class Spline : public SplineBase
{
public:
    typedef std::vector<length_type> LengthArray;
    typedef length_type length_type;
    #pragma region fields
protected:

    LengthArray lengths;

    void cacheLengths(float length_factor);
    index_type computeIndexInBounds(length_type length) const;
    #pragma endregion
public:

    explicit Spline(){}

    /** Calculates the position for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_percent(float t, Vector3 & c) const;

    /** Calculates Vector3(dx/dt, dy/dt, dz/dt) for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_hermite(float t, Vector3& hermite) const;

    /** Calculates the position for given segment Idx, and percent of segment length t
        @param t = partial_segment_length / whole_segment_length
        @param Idx - spline segment index, should be in range [first, last). */   
    void evaluate_percent(index_type Idx, float u, Vector3& c) const { SplineBase::evaluate_percent(Idx,u,c);}

    /** Caclulates Vector3(dx/dt, dy/dt, dz/dt) for index Idx, and percent of segment length t
        @param Idx - spline segment index, should be in range [first, last)
        @param t  - percent of spline segment length, assumes that t in range [0, 1]. */
    void evaluate_hermite(index_type Idx, float u, Vector3& c) const { SplineBase::evaluate_hermite(Idx,u,c);}

    // Assumes that t in range [0, 1]
    index_type computeIndexInBounds(float t) const;
    void computeIndex(float t, index_type& out_idx, float& out_u) const;

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void init_spline(const Vector3 * controls, const int N, EvaluationMode m, float length_factor);
    void init_cyclic_spline(const Vector3 * controls, const int N, EvaluationMode m, float length_factor, int cyclic_point);

    /** Returns length of the whole spline. */
    length_type length() const { return lengths[index_hi];}
    /** Returns length between given nodes. */
    length_type length(index_type first, index_type last) const { return lengths[last]-lengths[first];}
    length_type length(index_type Idx) const { return lengths[Idx];}

    void clear();
};

}

#include "spline.impl.h"
