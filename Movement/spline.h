
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

class QString;

namespace Movement {

class SplineBase
{
public:
    typedef int32 index_type;
    typedef QVector<Vector3> ControlArray;

    enum ErrorCodes {
        Uninitialized,
        SegmentIndexOutOfRange,
        PointIndexOutOfRange,
        SegmentCoeffOutOfRange,
        InitializationFailed,
    };

    enum EvaluationMode
    {
        ModeLinear,
        ModeCatmullrom,
        ModeEnd,
    };

    #pragma region fields
private:
    ControlArray points;

    index_type index_lo;
    index_type index_hi;
    EvaluationMode m_mode;

protected:
    inline void assertInitialized() const {
        assert_or_throw_msg(!empty(), ARGS(Exception<SplineBase,SplineBase::Uninitialized>), "spline is not initialized");
    }

    inline void assertSegmentIndexInRange(index_type index) const {
        assert_or_throw(index >= 0 && index < last(), ARGS(Exception<SplineBase,SegmentIndexOutOfRange>));
    }

    inline void assertCoeffInRange(float coeff) const {
        assert_or_throw(coeff >= 0.f && coeff <= 1.f, ARGS(Exception<SplineBase,SegmentCoeffOutOfRange>));
    }

private:
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

    /** Calculates position for given segment segmentIdx and percent of segment length u
        @param segmentIdx - spline segment index, should be in range [0, last)
        @param u - percent of segment length, assumes that u in range [0, 1]
     */
    Vector3 evaluatePosition(index_type segmentIdx, float u) const {
        assertInitialized();
        assertSegmentIndexInRange(segmentIdx);
        assertCoeffInRange(u);
        Vector3 pos;
        (this->*evaluators[m_mode])(index_lo + segmentIdx, u, pos);
        return pos;
    }

    /** Calculates derivation for given segment index segmentIdx and percent of segment length u.
        Function does not returns a unit vector!
        @param segmentIdx - spline segment index, should be in range [0, last)
        @param u - percent of spline segment length, assumes that u in range [0, 1]
     */
    Vector3 evaluateDerivative(index_type segmentIdx, float u) const {
        assertInitialized();
        assertSegmentIndexInRange(segmentIdx);
        assertCoeffInRange(u);
        Vector3 der;
        (this->*derivative_evaluators[m_mode])(index_lo + segmentIdx, u, der);
        return der;
    }

    /** Bounds for spline indexes.
        Point indexes are limited with [0, last] range.
        Segment indexes are limited with [0, last) range.
    */
    index_type last() const { return index_hi - index_lo;}

    bool empty() const { return index_lo == index_hi;}
    EvaluationMode mode() const { return m_mode;}

    const ControlArray& rawPoints() const { return points;}

    const Vector3& getPoint(index_type pointIdx) const {
        assertInitialized();
        assert_or_throw(0 <= pointIdx && pointIdx <= last(), ARGS(Exception<SplineBase,PointIndexOutOfRange>));
        return points[index_lo + pointIdx];
    }

    /** Initializes spline. Don't call other methods while spline not initialized
        otherwise Exception<SplineBase,Uninitialized> will be thrown.  */
    void initSpline(const Vector3 * controls, index_type count, EvaluationMode mode);
    /** Initializes spline. Don't call other methods while spline not initialized
        otherwise Exception<SplineBase,Uninitialized> will be thrown.
        cyclic_point parameter is a point index where spline end connects to that point, cyclic_point is [0, count). */
    void initCyclicSpline(const Vector3 * controls, index_type count, EvaluationMode mode, index_type cyclic_point);

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

    /** Calculates distance between boundary points of @segmentIdx segment. 
        @precision - segment length evaluation precision. asserts that precision > 0. */
    float segmentLength(index_type segmentIdx, uint32 precision = LengthPrecisionDefault) const {
        assertInitialized();
        assertSegmentIndexInRange(segmentIdx);
        return (this->*seglengths[m_mode])(index_lo + segmentIdx, precision);
    }

    QString toString() const;
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

    /** Computes a such spline segment @out_segmentIdx that 'lenghts[index] < t * lengthTotal < lenghts[index+1]'
        @param out_segmentIdx - spline segment index
        @param out_u - percent of spline segment length, out_u in range [0, 1].
        @param t - percent of spline's length, t in range [0, 1]. */
    void computeIndex(float t, index_type& out_segmentIdx, float& out_u) const;

    /**  Initializes lengths with SplineBase::segmentLength method. */    
    void initLengths(uint32 precision = SplineBase::LengthPrecisionDefault);

    /** Initializes lengths in some custom way
        Note that value returned by cacher must be greater or equal to previous value. */
    template<class T> inline void initLengths(T& cacher)
    {
        lengths.resize(last() + 1);
        index_type i = 1, N = lengths.size();
        while (i < N) {
            setLength(i, cacher(const_cast<const Spline&>(*this), i));
            ++i;
        }
    }

    /** Gets or sets length of the whole spline. */
    length_type lengthTotal() const { return lengths.last();}
    void lengthTotal(length_type value) {
        setLength(lengths.size()-1, value);
    }

    /** Returns length difference between @pointIdx and @pointIdxNext. */
    length_type lengthBetween(index_type pointIdx, index_type pointIdxNext) const {
        return lengths[pointIdxNext]-lengths[pointIdx];
    }

    /** Gets or sets length. Length is distance between 0 and @pointIdx spline points (or distance between 0 and low @segmentIndx point).*/
    length_type length(index_type pointIdx) const { return lengths[pointIdx];}
    void setLength(index_type pointIdx, length_type length) {
        assert_state(pointIdx == 0 || lengths[pointIdx-1] <= length);
        lengths[pointIdx] = length;
    }
};

}

#include "spline.impl.h"
