
/**
  file:         spline.h
  author:       SilverIce
  email:        slifeleaf@gmail.com
  created:      16:2:2011
*/

#pragma once

#include "framework/typedefs_p.h"
#include <G3D/Vector3.h>
#include <QtCore/QVector>

namespace Movement {

class SplineBase
{
public:
    typedef int32 index_type;
    typedef QVector<Vector3> ControlArray;

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeEnd,
    };

    #pragma region fields
protected:
    ControlArray points;

    index_type index_lo;
    index_type index_hi;
    EvaluationMode m_mode;

    inline void assertInitialized() const {
        assert_state(m_mode != ModeEnd);
    }

protected:
    void EvaluateLinear(index_type, float, Vector3&) const;
    void EvaluateCatmullRom(index_type, float, Vector3&) const;
    typedef void (SplineBase::*EvaluationMethtod)(index_type,float,Vector3&) const;
    static EvaluationMethtod evaluators[ModeEnd];

    void EvaluateDerivativeLinear(index_type, float, Vector3&) const;
    void EvaluateDerivativeCatmullRom(index_type, float, Vector3&) const;
    static EvaluationMethtod derivative_evaluators[ModeEnd];

    float SegLengthLinear(index_type,uint32) const;
    float SegLengthCatmullRom(index_type,uint32) const;
    typedef float (SplineBase::*SegLenghtMethtod)(index_type,uint32) const;
    static SegLenghtMethtod seglengths[ModeEnd];

    void InitLinear(const Vector3*, index_type, bool, index_type);
    void InitCatmullRom(const Vector3*, index_type, bool, index_type);
    typedef void (SplineBase::*InitMethtod)(const Vector3*, index_type, bool, index_type);
    static InitMethtod initializers[ModeEnd];

    #pragma endregion
public:

    explicit SplineBase() : index_lo(0), index_hi(0), m_mode(ModeEnd) {}

    /** Calculates position for given segment Idx and percent of segment length u
        @param Idx - spline segment index, should be in range [first, last)
        @param u - percent of segment length, assumes that u in range [0, 1]
     */
    Vector3 evaluatePosition(index_type Idx, float u) const {
        assertInitialized();
        Vector3 pos;
        (this->*evaluators[m_mode])(Idx,u,pos);
        return pos;
    }

    /** Calculates derivation in index Idx and percent of segment length u.
        Function does not returns a unit vector!
        @param Idx - spline segment index, should be in range [first, last)
        @param u - percent of spline segment length, assumes that u in range [0, 1]
     */
    Vector3 evaluateDerivative(index_type Idx, float u) const {
        assertInitialized();
        Vector3 der;
        (this->*derivative_evaluators[m_mode])(Idx,u,der);
        return der;
    }

    /**  Bounds for spline indexes. All indexes should be in range [first, last). */
    index_type first() const { return index_lo;}
    index_type last()  const { return index_hi;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return m_mode;}

    const ControlArray& getPoints() const { return points;}
    index_type getPointCount() const { return points.size();}
    const Vector3& getPoint(index_type i) const { return points[i];}

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void initSpline(const Vector3 * controls, index_type count, EvaluationMode m);
    void initCyclicSpline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point);

    /** As i can see there are a lot of ways how spline can be initialized
        would be no harm to have some custom initializers. */
    template<class Init> inline void initSpline(Init& initializer)
    {
        initializer(m_mode,points,index_lo,index_hi);
    }

    /** Segment length evaluation precision.
        Lesser value saves more performance in cost of lover precision.
        It's a iteration amount that need to be done to compute length.
        minimal value is 1. */
    enum LengthPrecision {
        LengthPrecisionDefault = 3,
        /* World of Warcraft client precision */
        LengthPrecisionWoWClient = 20,
    };

    /** Calculates distance between [i; i+1] points, assumes that index i is in bounds. */
    float segmentLength(index_type i, uint32 precision = LengthPrecisionDefault) const {
        assertInitialized();
        return (this->*seglengths[m_mode])(i, precision);
    }

    std::string ToString() const;
};

template<typename length_type>
class Spline : public SplineBase
{
public:
    typedef QVector<length_type> LengthArray;
    typedef length_type LenghtType;
protected:

    LengthArray lengths;

    index_type computeIndexFromLength(length_type length) const;

public:

    explicit Spline(){}

    /** Calculates the position for given t
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    Vector3 evaluatePosition(float t) const;

    /** Calculates derivation for given t. Function does not returns a unit vector!
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    Vector3 evaluateDerivative(float t) const;

    using SplineBase::evaluatePosition;
    using SplineBase::evaluateDerivative;

    /** Computes a such spline segment @index that 'lenghts[index] < t * lengthTotal < lenghts[index+1]'
        @param t - percent of spline's length, assumes that t in range [0, 1]. */
    index_type computeIndexInBounds(float t) const;

    /** Computes a such spline segment @index that 'lenghts[index] < t * lengthTotal < lenghts[index+1]'
        @param out_u - percent of spline segment length, out_u in range [0, 1].
        @param t - percent of spline's length, t in range [0, 1]. */
    void computeIndex(float t, index_type& out_idx, float& out_u) const;

    /**	Initializes spline. Don't call other methods while spline not initialized. */
    void initSpline(const Vector3 * controls, index_type count, EvaluationMode m) {
        SplineBase::initSpline(controls,count,m);
        lengths.resize(index_hi+1);
    }

    /**	Initializes cyclic spline. Don't call other methods while spline not initialized.
        @param cyclic_point - a such index of the path where path tail will smoothly transite to that index
    */
    void initCyclicSpline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point) {
        SplineBase::initCyclicSpline(controls,count,m,cyclic_point);
        lengths.resize(index_hi+1);
    }

    /**  Initializes lengths with SplineBase::segmentLength method. */    
    void initLengths(uint32 precision = SplineBase::LengthPrecisionDefault);

    /** Initializes lengths in some custom way
        Note that value returned by cacher must be greater or equal to previous value. */
    template<class T> inline void initLengths(T& cacher)
    {
        index_type i = index_lo;
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

    /** Returns length between two points. */
    length_type lengthBetween(index_type first, index_type last) const { return lengths[last]-lengths[first];}

    /** Gets or sets length. Length is distance between first and @i spline points. */
    length_type length(index_type i) const { return lengths[i];}
    void set_length(index_type i, length_type length) {
        assert_state(i > index_lo && i < (int32)lengths.size());
        assert_state(i == 0 || lengths[i-1] <= length);
        lengths[i] = length;
    }
};

}

#include "spline.impl.h"
