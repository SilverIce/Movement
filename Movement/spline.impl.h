#pragma once

namespace Movement
{
template<typename length_type> void Spline<length_type>::evaluate_percent( float t, Vector3 & c ) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_percent(Index, u, c);
}

template<typename length_type> void Spline<length_type>::evaluate_derivative(float t, Vector3& hermite) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_derivative(Index, u, hermite);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexFromLength(length_type length_) const
{
    index_type i = index_lo;
    index_type N = index_hi;
    while (i+1 < N && lengths[i+1] < length_)
        ++i;
    return i;
}

template<typename length_type> void Spline<length_type>::computeIndex(float t, index_type& index, float& u) const
{
    mov_assert(t >= 0.f && t <= 1.f);
    length_type length_ = t * length();
    index = computeIndexFromLength(length_);
    mov_assert(index < index_hi);
    u = (length_ - length(index)) / (float)length(index, index+1);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexInBounds( float t ) const
{
    mov_assert(t >= 0.f && t <= 1.f);
    return computeIndexFromLength(t * length());
}

template<typename length_type> void Spline<length_type>::initLengths()
{
    index_type i = index_lo;
    length_type length = 0;
    lengths.resize(index_hi+1);
    while(i < index_hi )
    {
        length += SegLength(i);
        lengths[++i] = length;
    }
}

}
