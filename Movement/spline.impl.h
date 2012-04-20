#pragma once

namespace Movement
{
template<typename length_type> Vector3 Spline<length_type>::evaluatePosition(float t) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    return evaluatePosition(Index, u);
}

template<typename length_type> Vector3 Spline<length_type>::evaluateDerivative(float t) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    return evaluateDerivative(Index, u);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexFromLength(length_type length_) const
{
    index_type i = 0;
    index_type N = lengths.size();
    while (i+1 < N && lengths[i+1] < length_)
        ++i;
    return i;
}

template<typename length_type> void Spline<length_type>::computeIndex(float t, index_type& index, float& u) const
{
    mov_assert(t >= 0.f && t <= 1.f);
    length_type length_ = t * lengthTotal();
    index = computeIndexFromLength(length_);
    mov_assert(index < last());
    u = (length_ - length(index)) / (float)lengthBetween(index, index+1);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexInBounds( float t ) const
{
    mov_assert(t >= 0.f && t <= 1.f);
    return computeIndexFromLength(t * lengthTotal());
}

template<typename length_type> void Spline<length_type>::initLengths(uint32 precision)
{
    struct LengthInitializer {
        float lengthSumm;
        uint32 precision;
        float operator()(SplineBase& spline, index_type i) {
            lengthSumm += spline.segmentLength(i-1, precision);
            return lengthSumm;
        }
    };
    LengthInitializer init = {0.f, precision};
    initLengths(init);
}

}
