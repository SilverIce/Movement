
/**
  file:         spline.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"
#include <G3D/Vector3.h>

namespace Movement {

class SplineBase
{
public:
    typedef int32 index_type;
    typedef std::vector<Vector3> ControlArray;

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeEnd,
    };

    #pragma region fields
protected:
    ControlArray points;

    enum {
        ModeUninitialized = 0xFF,
    };

    index_type index_lo;
    index_type index_hi;

    uint8 m_mode;

    enum{
        // could be modified, affects segment length evaluation precision
        // lesser value saves more performance in cost of lover precision
        // minimal value is 1
        // client's value is 20, blizzs use 2-3 steps to compute length
        STEPS_PER_SEGMENT = 3,
    };
    static_assert(STEPS_PER_SEGMENT > 0, "");

    inline void assertInitialized() const {
        assert_state(m_mode != ModeUninitialized);
    }

protected:
    void EvaluateLinear(index_type, float, Vector3&) const;
    void EvaluateCatmullRom(index_type, float, Vector3&) const;
    typedef void (SplineBase::*EvaluationMethtod)(index_type,float,Vector3&) const;
    static EvaluationMethtod evaluators[ModeEnd];

    void EvaluateDerivativeLinear(index_type, float, Vector3&) const;
    void EvaluateDerivativeCatmullRom(index_type, float, Vector3&) const;
    static EvaluationMethtod derivative_evaluators[ModeEnd];

    float SegLengthLinear(index_type) const;
    float SegLengthCatmullRom(index_type) const;
    typedef float (SplineBase::*SegLenghtMethtod)(index_type) const;
    static SegLenghtMethtod seglengths[ModeEnd];

    void InitLinear(const Vector3*, index_type, bool, index_type);
    void InitCatmullRom(const Vector3*, index_type, bool, index_type);
    typedef void (SplineBase::*InitMethtod)(const Vector3*, index_type, bool, index_type);
    static InitMethtod initializers[ModeEnd];

    #pragma endregion
public:

    explicit SplineBase() : m_mode(ModeUninitialized), index_lo(0), index_hi(0) {}

    /** Caclulates the position for given segment Idx, and percent of segment length t
        @param t - percent of segment length, assumes that t in range [0, 1]
        @param Idx - spline segment index, should be in range [first, last)
     */
    void evaluate_percent(index_type Idx, float u, Vector3& c) const {
        assertInitialized();
        (this->*evaluators[m_mode])(Idx,u,c);
    }

    /** Caclulates derivation in index Idx, and percent of segment length t
        @param Idx - spline segment index, should be in range [first, last)
        @param t  - percent of spline segment length, assumes that t in range [0, 1]
     */
    void evaluate_derivative(index_type Idx, float u, Vector3& hermite) const {
        assertInitialized();
        (this->*derivative_evaluators[m_mode])(Idx,u,hermite);
    }

    /**  Bounds for spline indexes. All indexes should be in range [first, last). */
    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return (EvaluationMode)m_mode;}

    const ControlArray& getPoints() const { return points;}
    index_type getPointCount() const { return points.size();}
    const Vector3& getPoint(index_type i) const { return points[i];}

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void init_spline(const Vector3 * controls, index_type count, EvaluationMode m);
    void init_cyclic_spline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point);

    /** As i can see there are a lot of ways how spline can be initialized
        would be no harm to have some custom initializers. */
    template<class Init> inline void init_spline(Init& initializer)
    {
        initializer(m_mode,points,index_lo,index_hi);
    }

    /** Calculates distance between [i; i+1] points, assumes that index i is in bounds. */
    float SegLength(index_type i) const {
        assertInitialized();
        return (this->*seglengths[m_mode])(i);
    }

    std::string ToString() const;
};

template<typename length_type>
class Spline : public SplineBase
{
public:
    typedef std::vector<length_type> LengthArray;
    typedef length_type LenghtType;
    #pragma region fields
protected:

    LengthArray lengths;

    index_type computeIndexFromLength(length_type length) const;

    void set_length(index_type i, length_type length) {
        assert_state(i == 0 || lengths[i-1] <= length);
        lengths[i] = length;
    }

    #pragma endregion
public:

    explicit Spline(){}

    /** Calculates the position for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_percent(float t, Vector3 & c) const;

    /** Calculates derivation for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    void evaluate_derivative(float t, Vector3& hermite) const;

    /** Calculates the position for given segment Idx, and percent of segment length t
        @param t = partial_segment_length / whole_segment_length
        @param Idx - spline segment index, should be in range [first, last). */   
    void evaluate_percent(index_type Idx, float u, Vector3& c) const { SplineBase::evaluate_percent(Idx,u,c);}

    /** Caclulates derivation for index Idx, and percent of segment length t
        @param Idx - spline segment index, should be in range [first, last)
        @param t  - percent of spline segment length, assumes that t in range [0, 1]. */
    void evaluate_derivative(index_type Idx, float u, Vector3& c) const { SplineBase::evaluate_derivative(Idx,u,c);}

    // Assumes that t in range [0, 1]
    index_type computeIndexInBounds(float t) const;
    void computeIndex(float t, index_type& out_idx, float& out_u) const;

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void init_spline(const Vector3 * controls, index_type count, EvaluationMode m) { SplineBase::init_spline(controls,count,m);}
    void init_cyclic_spline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point) { SplineBase::init_cyclic_spline(controls,count,m,cyclic_point);}

    /**  Initializes lengths with SplineBase::SegLength method. */    
    void initLengths();

    /** Initializes lengths in some custom way
        Note that value returned by cacher must be greater or equal to previous value. */
    template<class T> inline void initLengths(T& cacher)
    {
        index_type i = index_lo;
        lengths.resize(index_hi+1);
        while (i < index_hi) {
            set_length(i+1, cacher(*this, i));
            ++i;
        }
    }

    /** Gets or sets length of the whole spline. */
    length_type lengthTotal() const { return lengths[index_hi];}
    void lengthTotal(length_type value) {
        set_length(index_hi, value);
    }

    /** Returns length between given nodes. */
    length_type length(index_type first, index_type last) const { return lengths[last]-lengths[first];}
    length_type length(index_type Idx) const { return lengths[Idx];}
};

}

#include "spline.impl.h"
