
#include "spline.h"
#include "g3d\matrix4.h"

#include <assert.h>
#include <limits>

using namespace G3D;
namespace Movement{

SplinePure::InterpolatorPtr SplinePure::interpolators[SplineModeCount] =
{
    &SplinePure::InterpolateLinear,
    &SplinePure::InterpolateCatmullRom,
    &SplinePure::InterpolateBezier3,
};

SplinePure::SegLenghtPtr SplinePure::seglengths[SplineModeCount] =
{
    &SplinePure::SegLengthLinear,
    &SplinePure::SegLengthCatmullRom,
    &SplinePure::SegLengthBezier3,
};

SplinePure::InitPathPtr SplinePure::initializers[SplineModeCount] =
{
    //&SplinePure::InitLinear,
    &SplinePure::InitCatmullRom,    // we should use catmullrom initializer even for linear mode! (client's internal structure limitation) 
    &SplinePure::InitCatmullRom,
    &SplinePure::InitBezier3,
};

void SplinePure::evaluate_percent( float t, Vector3 & c ) const
{
    assert(t >= 0.f && t <= 1.f);

    float length_ = t * length();
    index_type Index = computeIndexInBounds(length_, t);
    assert(Index < index_hi);
    float u = (length_ - lengths[Index]) / (lengths[Index+1] - lengths[Index]);

    (this->*interpolators[m_mode])(Index, u, c);
}

SplinePure::index_type SplinePure::computeIndexInBounds( float length, float t ) const
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
    while (i+1 < N && lengths[i+1] < length)
    {
        ++i;
    }

    return i;
}

float SplinePure::SegLength( index_type Index ) const
{
    return (this->*seglengths[m_mode])(Index);
}

///////////
#pragma region evaluation methtods

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

inline void C_Evaluate(const Vector3 *vertice, float t, const Matrix4& coeffs, Vector3 &position)
{
    Vector3 tvec(t*t*t, t*t, t);
    int i = 0;
    double c;
    double x = 0, y = 0, z = 0;
    while ( i < 4 )
    {
        c = coeffs[0][i]*tvec.x + coeffs[1][i]*tvec.y + coeffs[2][i]*tvec.z + coeffs[3][i];

        x += c * vertice->x;
        y += c * vertice->y;
        z += c * vertice->z;

        ++i;
        ++vertice;
    }

    position.x = x;
    position.y = y;
    position.z = z;
}

void SplinePure::InterpolateLinear(index_type Idx, float u, Vector3& result) const
{
    assert(Index >= 0 && Index+1 < points.size());
    result = points[Idx] + (points[Idx+1] - points[Idx]) * u;
}

void SplinePure::InterpolateCatmullRom( index_type Index, float t, Vector3& result) const
{
    assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate(&points[Index - 1], t, s_catmullRomCoeffs, result);
}

void SplinePure::InterpolateBezier3(index_type Index, float t, Vector3& result) const
{
    Index *= 3u;
    assert(Index >= 0 && Index+3 < points.size());
    C_Evaluate(&points[Index], t, s_Bezier3Coeffs, result);
}

float SplinePure::SegLengthLinear(index_type i) const
{
    assert(Index >= 0 && Index+1 < points.size());
    return (points[i] - points[i+1]).length();
}

float SplinePure::SegLengthCatmullRom( index_type Index ) const
{
    assert(Index-1 >= 0 && Index+2 < points.size());

    Vector3 curPos, nextPos;
    const Vector3 * p = &points[Index - 1];
    curPos = nextPos = p[1];

    index_type i = 1;
    double length = 0;
    while (i <= STEPS_PER_SEGMENT)
    {
        C_Evaluate(p, float(i) / float(STEPS_PER_SEGMENT), s_catmullRomCoeffs, nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}

float SplinePure::SegLengthBezier3(index_type Index) const
{
    Index *= 3u;
    assert(Index >= 0 && Index+3 < points.size());

    Vector3 curPos, nextPos;
    const Vector3 * p = &points[Index];

    C_Evaluate(p, 0.f, s_Bezier3Coeffs, nextPos);
    curPos = nextPos;

    index_type i = 1;
    double length = 0;
    while (i <= STEPS_PER_SEGMENT)
    {
        C_Evaluate(p, float(i) / float(STEPS_PER_SEGMENT), s_Bezier3Coeffs, nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}
#pragma endregion

SplinePure::SplinePure() : m_mode(SplineModeLinear)
{
    index_lo = 0;
    index_hi = 0;
    points_count = 0;
}

void SplinePure::init_path( const Vector3 * controls, const int count, SplineMode m )
{
    m_mode = m;
    points_count = count;
    cyclic = false;

    (this->*initializers[m_mode])(controls, count, cyclic, 0);
    cacheLengths();
}

void SplinePure::init_cyclic_path( const Vector3 * controls, const int count, SplineMode m, int cyclic_point )
{
    m_mode = m;
    points_count = count;
    cyclic = true;

    (this->*initializers[m_mode])(controls, count, cyclic, cyclic_point);
    cacheLengths();
}

void SplinePure::InitLinear( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    assert(count >= 2);
    const int real_size = count + 1;

    points.resize(real_size);
    lengths.resize(real_size,0.f);

    memcpy(&points[0],controls, sizeof(Vector3) * count);

    // index 0 and last two\one indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate methtod work
    if (cyclic)
        points[count] = controls[cyclic_point];
    else
        points[count] = controls[count-1];

    index_lo = 0;
    index_hi = cyclic ? count : (count - 1);
}

void SplinePure::InitCatmullRom( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    const int real_size = count + (cyclic ? (1+2) : (1+1));

    points.resize(real_size);
    //times.resize(real_size,0);
    lengths.resize(real_size,0.f);

    int lo_idx = 1;
    int high_idx = lo_idx + count - 1; 

    memcpy(&points[lo_idx],controls, sizeof(Vector3) * count);

    // index 0 and last two\one indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate methtod work
    if (cyclic)
    {
        if (cyclic_point == 0)
            points[0] = controls[count-1];
        else
            points[0] = controls[0];

        points[high_idx+1] = controls[cyclic_point];
        points[high_idx+2] = controls[cyclic_point+1];
    }
    else
    {
        points[0] = controls[0];
        points[high_idx+1] = controls[count-1];
    }

    index_lo = lo_idx;
    index_hi = high_idx + (cyclic ? 1 : 0);
}

void SplinePure::InitBezier3( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    index_type c = count / 3u * 3u;
    index_type t = c / 3u;

    points.resize(c);
    lengths.resize(t,0.f);
    memcpy(&points[0],controls, sizeof(Vector3) * c);

    index_lo = 0;
    index_hi = t-1;
    //assert(points.size() % 3 == 0);
}

void SplinePure::cacheLengths()
{
    index_type i = index_lo;
    double length = 0;
    while(i < index_hi )
    {
        float l = SegLength(i);

        // little trick:
        // there are some paths provided by DB where all points have same coords!
        // as a result - SplinePure interpolates position with NaN coords
        if ( l == 0.f )
            l = std::numeric_limits<float>::min();

        length += l;
        lengths[i+1] = length;
        ++i;
    }
}

void SplinePure::clear()
{
    index_lo = 0;
    index_hi = 0;
    points_count = 0;

    points.clear();
    lengths.clear();
}

void SplinePure::erase( index_type i )
{
    assert(false && "SplinePure::erase is in dev. state, it shouldn't be used");
    assert(index_lo >= i && i <= index_hi);

    PointsArray copy;
    copy.reserve(points_count-1);

    std::vector<Vector3>::iterator it = points.begin()+index_lo;
    copy.insert(copy.end(), it, it + i);
    copy.insert(copy.end(), it + i, it + points_count);


    --points_count;
    (this->*initializers[m_mode])(&copy[0], copy.size(), cyclic, 0);
    cacheLengths();
}

SplineLive::index_type SplineLive::computeIndexInBounds( float length, float t ) const
{
    index_type i = m_current_node;
    index_type N = index_hi;
    while (i+1 < N && lengths[i+1] < length)
        ++i;
    return i;
}

void SplineLive::evaluate_percent( float t, Vector3 & c )
{
    assert(t >= 0.f && t <= 1.f);
    float length_ = t * length();
    m_current_node = computeIndexInBounds(length_, t);
    assert(Index < index_hi);

    float u = (length_ - lengths[m_current_node]) / (lengths[m_current_node+1] - lengths[m_current_node]);

    (this->*interpolators[m_mode])(m_current_node, u, c);
}

void SplineLive::init_path( const Vector3 * controls, const int N, SplineMode m )
{
    SplinePure::init_path(controls, N, m);
    reset_progress();
}

void SplineLive::init_cyclic_path( const Vector3 * controls, const int N, SplineMode m, int cyclic_point )
{
    SplinePure::init_cyclic_path(controls, N, m, cyclic_point);
    reset_progress();
}

}
