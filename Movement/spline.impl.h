#pragma once

#include <limits>

namespace Movement
{
template<typename length_type> void Spline<length_type>::evaluate_percent( float t, Vector3 & c ) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_percent(Index, u, c);
}

template<typename length_type> void Spline<length_type>::evaluate_hermite(float t, Vector3& hermite) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_hermite(Index, u, hermite);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexInBounds(length_type length_) const
{
// Temporary disabled: causes infinite loop with t = 1.f
/*
    index_type hi = index_hi;
    index_type lo = index_lo;

    index_type i = lo + (float)(hi - lo) * t;

    while ((lengths[i] > length) || (lengths[i + 1] <= length))
    {
        if (lengths[i] > length)
            hi = i - 1; // too big
        else if (lengths[i + 1] <= length)
            lo = i + 1; // too small

        i = (hi + lo) / 2;
    }*/

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
    index = computeIndexInBounds(length_);
    mov_assert(index < index_hi);
    u = (length_ - length(index)) / (float)length(index, index+1);
}

template<typename length_type> SplineBase::index_type Spline<length_type>::computeIndexInBounds( float t ) const
{
    mov_assert(t >= 0.f && t <= 1.f);
    return computeIndexInBounds(t * length());
}

template<typename length_type> void Spline<length_type>::cacheLengths(float length_factor)
{
    index_type i = index_lo;
    length_type length = 0;
    lengths.resize(index_hi+1);
    while(i < index_hi )
    {
        float l = SegLength(i);

        // little trick:
        // there are some paths provided by DB where all points have same coords!
        // as a result - Spline interpolates position with NaN coords
        if ( l == 0.f )
            l = std::numeric_limits<float>::min();

        length += l * length_factor;
        lengths[i+1] = length;
        ++i;
    }
}

template<typename length_type> void Spline<length_type>::clear()
{
    SplineBase::clear();
    lengths.clear();
}

}
