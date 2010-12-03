#include "g3d_based.h"

#include "g3d\vector4.h"

using namespace G3D;

void GD3_spline::evaluate( float s, Vector3& position, const Matrix4& basis ) const
{
    debugAssertM(control.size() == time.size(), "Corrupt spline: wrong number of control points.");

    /*
    @cite http://www.gamedev.net/reference/articles/article1497.asp 
    Derivation of basis matrix follows.

    Given control points with positions p[i] at times t[i], 0 <= i <= 3, find the position 
    at time t[1] <= s <= t[2].

    Let u = s - t[0]
    Let U = [u^0 u^1 u^2 u^3] = [1 u u^2 u^3]
    Let dt0 = t[0] - t[-1]
    Let dt1 = t[1] - t[0]
    Let dt2 = t[2] - t[1]
    */

    // Index of the first control point (i.e., the u = 0 point)
    int i = 0;
    // Fractional part of the time
    float u = 0;

    computeIndex(s, i, u);

    Vector3 p[4];
    float   t[4];
    getControls(i - 1, t, p, 4);
    float dt0 = t[1] - t[0];
    float dt1 = t[2] - t[1];
    float dt2 = t[3] - t[2];

    // Powers of u
    Vector4 uvec((float)(u*u*u), (float)(u*u), (float)u, 1.0f);

    // Compute the weights on each of the control points.
    const Vector4& weights = uvec * basis;

    // Compute the weighted sum of the neighboring control points.
    Vector3 sum;

    const Vector3& p0 = p[0];
    const Vector3& p1 = p[1];
    const Vector3& p2 = p[2];
    const Vector3& p3 = p[3];

    const Vector3& dp0 = p1 - p0;
    const Vector3& dp1 = p2 - p1;
    const Vector3& dp2 = p3 - p2;

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

    position = 
        tan1 * weights[0]+
        p1  * weights[1] +
        p2  * weights[2] +
        tan2 * weights[3]; 
}

void GD3_spline::append( Vector3 &v )
{
    control.push_back(v);
    int N = control.size();
    if (N > 1)
    {
        float t = (control[N-1]-control[N-2]).length() / 7.0f * 1000.f;
        this->time.push_back( time.last() + t);
    }
    else
        time.push_back(0.0f);
}