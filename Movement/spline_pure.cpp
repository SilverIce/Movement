
#include "spline_pure.h"
#include "g3d\matrix4.h"
#include "g3d\vector4.h"
#include "outLog.h"

using namespace G3D;

void InterpolateLinear(int Idx, float u, const SplinePure&, Vector3&);
void InterpolateCatmullRom(int Idx, float u, const SplinePure&, Vector3&);
void InterpolateBezier(int Idx, float u, const SplinePure&, Vector3&);

typedef void (*InterpolationMethtod)(int, float, const SplinePure&, Vector3&);

const InterpolationMethtod interpolators[SplineModeCount]=
{
    &InterpolateLinear,
    &InterpolateCatmullRom,
    &InterpolateBezier,    //not implemented
};

void SplinePure::evaluate( int time, Vector3 & c ) const
{
    assert(times.size() == points.size());

    int Index = 0;
    float u = 0.f;
    computeIndex(0, Index, time, u);

    (*interpolators[mode])(Index, u, *this, c);

    sLog.write("%f   %f", c.x, c.y);
}

void SplinePure::computeIndex( int lastIndex, int& Index, int &X, float &percent) const
{
    int N = times.size();

    if (!inBounds(X))
    {
        int high = hight_bound();
        int low = low_bound();

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
            X = X % high;    // x now in range [ 0; duration )

            if( X < low)    // X is between last and first point: [0; times[0]) or ( times[N-1]; times[0] )
            {
                Index = N - 1; // its cycled spline, so '-1' equals to 'N - 1'
                percent = float(X) / float(low);
                return;
            }

            lastIndex = 0;
        }
    }

    assert(Index+1 < N);
    Index = computeIndexInBounds(lastIndex, X);
    percent = float(X - times[Index]) / float(times[Index+1] - times[Index]);

    assert(X <= duration());
}

template<class Y, class X> inline Y interporate(const Y * a, X ia,  X I)
{
    return ( a[0] + (a[1] - a[0]) / 1 * (I - ia) );
}

void SplinePure::getControl(int i, int& t, Vector3& c ) const
{
    int N = times.size();
    if (i >= 0 && i < N) // normal, in bounds case
    {
        t = times[i];
        c = points[i];
        return;
    }

    // indexes that not in bounds are in range  (-N; N+N) -
    // this simplifies my calculations a lot
    assert( -N < i && i < N+N );
    int last_i = N - 1;
    if (cyclic)
    {
//         int normalized = i % N;
//         if (i < 0)
//             normalized = N + normalized;
// 
//         if(i >= 0)
//             t = times[last_i] + times[normalized];
//         else
//             t = -times[last_i] + times[normalized];
// 
//         c = points[normalized];

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
        if(i >= 0)
        {
            c = interporate<Vector3,int>(&points[last_i-1], last_i-1, i);
            t = interporate<int,int>(&times[last_i-1], last_i-1, i);
        }
        else
        {
            c = interporate<Vector3,int>(&points[0], 0, i);
            t = interporate<int,int>(&times[0], 0, i);
        }
    }
}

void SplinePure::append( Vector3 control )
{
    points.push_back(control);
}

int SplinePure::computeIndexInBounds( int lastIdx, const int time_passed_delta ) const
{
    uint32 N = times.size();
    while (lastIdx+1 < N && times[lastIdx+1] < time_passed_delta)
    {
        ++lastIdx;
    }
    return lastIdx;
}
///////////

static const G3D::Matrix4 s_catmullRomCoeffs(
    -0.5f, 1.5f,-1.5f, 0.5f,
    1.f, -2.5f, 2.f, -0.5f,
    -0.5f, 0.f,  0.5f, 0.f,
    0.f,  1.f,  0.f,  0.f);

static const G3D::Matrix4 s_bezierCoeffs(
    -1.f,  3.f, -3.f, 1.f,
    3.f, -6.f,  3.f, 0.f,
    -3.f,  3.f,  0.f, 0.f,
    1.f,  0.f,  0.f, 0.f);


void InterpolateLinear(int Idx, float u, const SplinePure & spline, Vector3& result)
{
    if ( Idx+1 < spline.times.size() )
    {
        result = spline.points[Idx] + (spline.points[Idx+1] - spline.points[Idx]) * u;
    }
    else
    {
        if (!spline.cyclic)
            result = spline.points.back();// Idx points at last point
        else
        {
            result = spline.points[Idx] + (spline.points[0] - spline.points[Idx]) * u;
        }
    }
}

void InterpolateCatmullRom(int i, float u, const SplinePure & spline, Vector3& result)
{
    Vector3 p[4];
    int     t[4];
    spline.getControls(i - 1, t, p, 4);
    float dt0 = t[1] - t[0];
    float dt1 = t[2] - t[1];
    float dt2 = t[3] - t[2];

    const Vector3& p0 = p[0];
    const Vector3& p1 = p[1];
    const Vector3& p2 = p[2];
    const Vector3& p3 = p[3];

    const Vector3& dp0 = p1 - p0;
    const Vector3& dp1 = p2 - p1;
    const Vector3& dp2 = p3 - p2;

    // Powers of u
    G3D::Vector4 uvec((float)(u*u*u), (float)(u*u), (float)u, 1.0f);

    // Compute the weights on each of the control points.
    const G3D::Vector4& weights = uvec * s_catmullRomCoeffs;

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

void InterpolateBezier(int, float, const SplinePure&, Vector3&)
{
    assert(false);
}

