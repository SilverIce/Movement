
#include "spline_pure.h"
#include "g3d\matrix4.h"
#include "g3d\vector4.h"
#include "outLog.h"

using namespace G3D;

SplinePure::InterpolatorPtr SplinePure::interpolators[SplineModeCount] =
{
    &SplinePure::InterpolateLinear,
    &SplinePure::InterpolateG3DCatmullRom,
    &SplinePure::InterpolateCatmullRom,
    &SplinePure::InterpolateBezier3,
};

SplinePure::SegLenghtPtr SplinePure::seglengths[SplineModeCount] =
{
    &SplinePure::SegLengthLinear,
    &SplinePure::SegLengthG3DCatmullRom,
    &SplinePure::SegLengthCatmullRom,
    &SplinePure::SegLengthBezier3,
};


void SplinePure::evaluate( time_type time, Vector3 & c ) const
{
    assert(time >= 0 && times.size() == points.size());

    int Index = 0;
    float u = 0.f;
    computeIndex(0, Index, time, u);

    (this->*interpolators[mode])(Index, u, c);

    sLog.write("%f   %f", c.x, c.y);
}

void SplinePure::computeIndex( index_type lastIndex, index_type& Index, time_type &X, float &percent) const
{
    index_type N = times.size();

    time_type high = hight_bound();
    time_type low = low_bound();

    if (low > X || X >= high)  // X is out of bounds?
    {
        if (!cyclic)
        {
            percent = 0.f;
            if( X < low)
            {
                Index = 0;
                X = low;
            }
            else
            {
                Index = N - 1;
                X = high;
            }
            return;
        }
        else
        {
            X = X % duration();    // X now in range [ 0; duration )

            if( X >= high)    // X in range  [ high; duration )
            {
                Index = N - 1;
                percent = float(X - high) / float(finalInterval);
                return;
            }

            lastIndex = 0;
        }
    }

    Index = computeIndexInBounds(lastIndex, X);
    assert(Index + 1 < N);
    percent = float(X - times[Index]) / float(times[Index+1] - times[Index]);
}

template<class Y, class X> inline Y interporate(const Y * a, X ia,  X I)
{
    return ( a[0] + (a[1] - a[0]) / 1 * (I - ia) );
}

void SplinePure::getControl(index_type i, time_type& t, Vector3& c ) const
{
    index_type N = times.size();
    if (i >= 0 && i < N) // normal, in bounds case
    {
        t = times[i];
        c = points[i];
        return;
    }

    if (cyclic)
    {
        if(i >= 0)
        {
            int wraps = i / N;
            int j = i % N;
            t = wraps * duration() + times[j];
            c = points[j];
        }
        else
        {
            int wraps = (N + 1 - i) / N;                    
            int j = (i + wraps * N) % N;
            t = times[j] - wraps * duration();
            c = points[j];
        }
    } 
    else
    {
        index_type last_i = N - 1;
        if(i >= 0)
        {
            c = interporate<Vector3,index_type>(&points[last_i-1], last_i-1, i);
            t = interporate<time_type,index_type>(&times[last_i-1], last_i-1, i);
        }
        else
        {
            c = interporate<Vector3,index_type>(&points[0], 0, i);
            t = interporate<time_type,index_type>(&times[0], 0, i);
        }
    }
}

SplinePure::index_type SplinePure::computeIndexInBounds( index_type lastIdx, const time_type time_passed_delta ) const
{
    index_type N = times.size();
    while (lastIdx+1 < N && times[lastIdx+1] < time_passed_delta)
    {
        ++lastIdx;
    }
    return lastIdx;
}


float SplinePure::SegLength( index_type Index ) const
{
    return (this->*seglengths[mode])(Index);
}

///////////

static const G3D::Matrix4 s_catmullRomCoeffs(
    -0.5f, 1.5f,-1.5f, 0.5f,
    1.f, -2.5f, 2.f, -0.5f,
    -0.5f, 0.f,  0.5f, 0.f,
    0.f,  1.f,  0.f,  0.f);

static const G3D::Matrix4 s_Bezier3Coeffs(
    -1.f,  3.f, -3.f, 1.f,
    3.f, -6.f,  3.f, 0.f,
    -3.f,  3.f,  0.f, 0.f,
    1.f,  0.f,  0.f, 0.f);

static const G3D::Matrix4 g3d_catmullrom_basis(
    0.5f, 2.f, -2.f, 0.5f,
    -1.f, -3.f, 3.f, -0.5f,
    0.5f, 0.f, 0.f, 0.f,
    -0.f, 1.f, 0.f, 0.f);

void C_Evaluate(const Vector3 *vertice, float t, const Matrix4& coeffs, Vector3 &position)
{
    int _4_cycles = 4;
    int i = 0;

    position = Vector3::zero();

    double c;
    while ( i < _4_cycles )
    {
        c = (((coeffs[0][i] * t + coeffs[1][i]) * t) + coeffs[2][i]) * t + coeffs[3][i];
       
        position += c * (*vertice);

        ++i;
        ++vertice;
    }
}

void SplinePure::InterpolateLinear(index_type Idx, float u, Vector3& result) const
{
    time_type unused;
    Vector3 pos1, pos2;
    getControl(Idx, unused, pos1);
    getControl(Idx+1, unused, pos2);
    result = pos1 + (pos2 - pos1) * u;
}

inline void InterpolateCatmullRom2(const Vector3* p, const SplinePure::time_type* t, float u, Vector3& result)
{
    float dt0 = t[1] - t[0];
    float dt1 = t[2] - t[1];
    float dt2 = t[3] - t[2];

    const Vector3& p0 = p[0];
    const Vector3& p1 = p[1];
    const Vector3& p2 = p[2];
    const Vector3& p3 = p[3];

    const Vector3& dp2 = p3 - p2;
    const Vector3& dp1 = p2 - p1;
    const Vector3& dp0 = p1 - p0;

    // Powers of u
    G3D::Vector4 uvec((float)(u*u*u), (float)(u*u), (float)u, 1.0f);

    // Compute the weights on each of the control points.
    const G3D::Vector4& weights = uvec * g3d_catmullrom_basis;

    // The factor of 1/2 from averaging two time intervals is 
    // already factored into the basis

    // tan1 = (dp0 / dt0 + dp1 / dt1) * ((dt0 + dt1) * 0.5);
    // The last term normalizes for unequal time intervals
    float x = (dt0 + dt1) * 0.5f;
    float n0 = x / dt0;
    float n1 = x / dt1;
    float n2 = x / dt2;
    const Vector3& dp1n1 = dp1 * n1;
    const Vector3& tan1 = dp0 * n0 + dp1n1;
    const Vector3& tan2 = dp1n1 + dp2 * n2;

    result = 
        tan1 * weights[0]+
        p1  * weights[1] +
        p2  * weights[2] +
        tan2 * weights[3]; 
}

void SplinePure::InterpolateG3DCatmullRom(index_type i, float u, Vector3& result) const
{
    Vector3     p[4];
    time_type   t[4];
    getControls(i - 1, t, p, 4);
    InterpolateCatmullRom2(p, t, u, result);
}

void SplinePure::InterpolateCatmullRom( index_type i, float t, Vector3& result) const
{
    Vector3     p[4];
    time_type   unused[4];
    getControls(i - 1, unused, p, 4);

    C_Evaluate(p, t, s_catmullRomCoeffs, result);
}

void SplinePure::InterpolateBezier3(index_type i, float t, Vector3& result) const
{
    Vector3     p[4];
    time_type   unused[4];
    getControls(i - 1, unused, p, 4);

    C_Evaluate(p, t, s_Bezier3Coeffs, result);
}

float SplinePure::SegLengthLinear(index_type i) const
{
    index_type unused;
    Vector3 pos1, pos2;
    getControl(i, unused, pos1);
    getControl(i+1, unused, pos2);
    return (pos2 - pos1).length();
}

#define STEPS_PER_SEGMENT   20

// egg or chicken?
// seems to get the length of one segment need know whole spline length or length between spline vertices..

float SplinePure::SegLengthG3DCatmullRom(index_type Index) const
{
    Vector3 curPos, nextPos;
    Vector3     p[4];
    time_type   t[4];
    getControls(Index - 1, t, p, 4);
    InterpolateCatmullRom2(p, t, 0.f, curPos);
    nextPos = curPos;

    index_type N = STEPS_PER_SEGMENT;
    index_type i = 1;
    float length = 0;
    while (i < N)
    {
        InterpolateCatmullRom2(p, t, float(i) / float(N), nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}

float SplinePure::SegLengthCatmullRom( index_type Index ) const
{
    Vector3 curPos, nextPos;
    Vector3     p[4];
    time_type   t[4];
    getControls(Index - 1, t, p, 4);

    curPos = nextPos = p[1];

    index_type N = STEPS_PER_SEGMENT;
    index_type i = 1;
    float length = 0;
    while (i < N)
    {
        C_Evaluate(p, float(i) / float(N), s_catmullRomCoeffs, nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}

float SplinePure::SegLengthBezier3(index_type Index) const
{
    Vector3 curPos, nextPos;
    Vector3     p[4];
    time_type   t[4];
    getControls(Index - 1, t, p, 4);

    curPos = nextPos = p[1];

    index_type N = STEPS_PER_SEGMENT;
    index_type i = 1;
    float length = 0;
    while (i < N)
    {
        C_Evaluate(p, float(i) / float(N), s_Bezier3Coeffs, nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}

SplinePure::SplinePure() : cyclic(false), mode(SplineModeLinear), times(2), finalInterval(0)
{

}




