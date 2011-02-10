
#include "spline.h"
#include <limits>
#include <sstream>
#include "G3D\Matrix4.h"

namespace Movement{

Spline::EvaluationMethtod Spline::evaluators[Spline::ModesCount] =
{
    &Spline::EvaluateLinear,
    &Spline::EvaluateCatmullRom,
    &Spline::EvaluateBezier3,
};

Spline::EvaluationMethtod Spline::hermite_evaluators[Spline::ModesCount] =
{
    &Spline::EvaluateHermiteLinear,
    &Spline::EvaluateHermiteCatmullRom,
    &Spline::EvaluateHermiteBezier3,
};

Spline::SegLenghtMethtod Spline::seglengths[Spline::ModesCount] =
{
    &Spline::SegLengthLinear,
    &Spline::SegLengthCatmullRom,
    &Spline::SegLengthBezier3,
};

Spline::InitMethtod Spline::initializers[Spline::ModesCount] =
{
    //&Spline::InitLinear,
    &Spline::InitCatmullRom,    // we should use catmullrom initializer even for linear mode! (client's internal structure limitation)
    &Spline::InitCatmullRom,
    &Spline::InitBezier3,
};

void Spline::evaluate_percent( float t, Vector3 & c ) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_percent(Index, u, c);
}

void Spline::evaluate_hermite(float t, Vector3& hermite) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);
    evaluate_hermite(Index, u, hermite);
}

void Spline::evaluate_percent_and_hermite(float t, Vector3 & c, Vector3& hermite) const
{
    index_type Index;
    float u;
    computeIndex(t, Index, u);

    (this->*evaluators[m_mode])(Index, u, c);
    (this->*hermite_evaluators[m_mode])(Index, u, hermite);
}

Spline::index_type Spline::computeIndexInBounds( float length_, float t ) const
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

void Spline::computeIndex(float t, index_type& index, float& u) const
{
    mov_assert(t >= 0.f && t <= 1.f);

    float length_ = t * length();
    index = computeIndexInBounds(length_, t);
    mov_assert(index < index_hi);
    u = (length_ - length(index)) / segment_length(index);
}

Spline::index_type Spline::computeIndexInBounds( float t ) const
{
    float length_ = t * length();
    index_type i = index_lo;
    index_type N = index_hi;
    while (i+1 < N && lengths[i+1] < length_)
        ++i;

    return i;
}

float Spline::SegLength( index_type Index ) const
{
    return (this->*seglengths[m_mode])(Index);
}

///////////
#pragma region evaluation methtods

using G3D::Matrix4;
static const Matrix4 s_catmullRomCoeffs(
    -0.5f, 1.5f,-1.5f, 0.5f,
    1.f, -2.5f, 2.f, -0.5f,
    -0.5f, 0.f,  0.5f, 0.f,
    0.f,  1.f,  0.f,  0.f);

static const Matrix4 s_Bezier3Coeffs(
    -1.f,  3.f, -3.f, 1.f,
    3.f, -6.f,  3.f, 0.f,
    -3.f,  3.f,  0.f, 0.f,
    1.f,  0.f,  0.f, 0.f);

static const Matrix4 g3d_catmullrom_basis(
    0.5f, 2.f, -2.f, 0.5f,
    -1.f, -3.f, 3.f, -0.5f,
    0.5f, 0.f, 0.f, 0.f,
    -0.f, 1.f, 0.f, 0.f);

/*  classic view:
inline void C_Evaluate(const Vector3 *vertice, float t, const float (&matrix)[4][4], Vector3 &position)
{
    Vector3 tvec(t*t*t, t*t, t);
    int i = 0;
    double c;
    double x = 0, y = 0, z = 0;
    while ( i < 4 )
    {
        c = matrix[0][i]*tvec.x + matrix[1][i]*tvec.y + matrix[2][i]*tvec.z + matrix[3][i];

        x += c * vertice->x;
        y += c * vertice->y;
        z += c * vertice->z;

        ++i;
        ++vertice;
    }

    position.x = x;
    position.y = y;
    position.z = z;
}*/

using G3D::Matrix4;

inline void C_Evaluate(const Vector3 *vertice, float t, const Matrix4& matr, Vector3 &result)
{
    Vector4 tvec(t*t*t, t*t, t, 1.f);
    Vector4 weights(tvec * matr);

    result = vertice[0] * weights[0] + vertice[1] * weights[1]
           + vertice[2] * weights[2] + vertice[3] * weights[3];
}

inline void C_Evaluate_Hermite(const Vector3 *vertice, float t, const Matrix4& matr, Vector3 &result)
{
    Vector4 tvec(3.f*t*t, 2.f*t, 1.f, 0.f);
    Vector4 weights(tvec * matr);

    result = vertice[0] * weights[0] + vertice[1] * weights[1]
           + vertice[2] * weights[2] + vertice[3] * weights[3];
}

void Spline::EvaluateLinear(index_type Idx, float u, Vector3& result) const
{
    mov_assert(Idx >= 0 && Idx+1 < points.size());
    result = points[Idx] + (points[Idx+1] - points[Idx]) * u;
}

void Spline::EvaluateCatmullRom( index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate(&points[Index - 1], t, s_catmullRomCoeffs, result);
}

void Spline::EvaluateBezier3(index_type Index, float t, Vector3& result) const
{
    Index *= 3u;
    mov_assert(Index >= 0 && Index+3 < points.size());
    C_Evaluate(&points[Index], t, s_Bezier3Coeffs, result);
}

void Spline::EvaluateHermiteLinear(index_type Index, float, Vector3& result) const
{
    mov_assert(Index >= 0 && Index+1 < points.size());
    result = points[Index+1] - points[Index];
}

void Spline::EvaluateHermiteCatmullRom(index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate_Hermite(&points[Index - 1], t, s_catmullRomCoeffs, result);
}

void Spline::EvaluateHermiteBezier3(index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate_Hermite(&points[Index - 1], t, s_Bezier3Coeffs, result);
}

float Spline::SegLengthLinear(index_type i) const
{
    mov_assert(i >= 0 && i+1 < points.size());
    return (points[i] - points[i+1]).length();
}

float Spline::SegLengthCatmullRom( index_type Index ) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());

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

float Spline::SegLengthBezier3(index_type Index) const
{
    Index *= 3u;
    mov_assert(Index >= 0 && Index+3 < points.size());

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

Spline::Spline() : m_mode(ModeLinear), index_lo(0), index_hi(0), points_count(0), cyclic(false)
{
}

void Spline::init_spline( const Vector3 * controls, const int count, EvaluationMode m )
{
    m_mode = m;
    points_count = count;
    cyclic = false;

    (this->*initializers[m_mode])(controls, count, cyclic, 0);
    cacheLengths();
}

void Spline::init_cyclic_spline( const Vector3 * controls, const int count, EvaluationMode m, int cyclic_point )
{
    m_mode = m;
    points_count = count;
    cyclic = true;

    (this->*initializers[m_mode])(controls, count, cyclic, cyclic_point);
    cacheLengths();
}

void Spline::InitLinear( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    mov_assert(count >= 2);
    const int real_size = count + 1;

    points.resize(real_size);
    lengths.resize(real_size,0.f);

    memcpy(&points[0],controls, sizeof(Vector3) * count);

    // first and last two indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate and C_Evaluate_Hermite methtod work
    if (cyclic)
        points[count] = controls[cyclic_point];
    else
        points[count] = controls[count-1];

    index_lo = 0;
    index_hi = cyclic ? count : (count - 1);
}

void Spline::InitCatmullRom( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    const int real_size = count + (cyclic ? (1+2) : (1+1));

    points.resize(real_size);
    lengths.resize(real_size,0.f);

    int lo_idx = 1;
    int high_idx = lo_idx + count - 1;

    memcpy(&points[lo_idx],controls, sizeof(Vector3) * count);

    // first and last two indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate and C_Evaluate_Hermite methtod work
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

void Spline::InitBezier3( const Vector3* controls, const int count, bool cyclic, int cyclic_point )
{
    index_type c = count / 3u * 3u;
    index_type t = c / 3u;

    points.resize(c);
    lengths.resize(t,0.f);
    memcpy(&points[0],controls, sizeof(Vector3) * c);

    index_lo = 0;
    index_hi = t-1;
    //mov_assert(points.size() % 3 == 0);
}

void Spline::cacheLengths()
{
    index_type i = index_lo;
    double length = 0;
    while(i < index_hi )
    {
        float l = SegLength(i);

        // little trick:
        // there are some paths provided by DB where all points have same coords!
        // as a result - Spline interpolates position with NaN coords
        if ( l == 0.f )
            l = std::numeric_limits<float>::min();

        length += l;
        lengths[i+1] = length;
        ++i;
    }
}

void Spline::clear()
{
    index_lo = 0;
    index_hi = 0;
    points_count = 0;

    points.clear();
    lengths.clear();
}

std::string Spline::ToString() const
{
    std::stringstream str;

    index_type count = this->points.size();
    str << "points count: " << count << std::endl;
    for (index_type i = 0; i < count; ++i)
        str << "point " << i << " : " << points[i].toString() << std::endl;

    return str.str();
}

}
