
#include "spline.h"
#include <sstream>
#include "G3D\Matrix4.h"

namespace Movement{

SplineBase::EvaluationMethtod SplineBase::evaluators[SplineBase::ModesCount] =
{
    &SplineBase::EvaluateLinear,
    &SplineBase::EvaluateCatmullRom,
    &SplineBase::EvaluateBezier3,
};

SplineBase::EvaluationMethtod SplineBase::hermite_evaluators[SplineBase::ModesCount] =
{
    &SplineBase::EvaluateHermiteLinear,
    &SplineBase::EvaluateHermiteCatmullRom,
    &SplineBase::EvaluateHermiteBezier3,
};

SplineBase::SegLenghtMethtod SplineBase::seglengths[SplineBase::ModesCount] =
{
    &SplineBase::SegLengthLinear,
    &SplineBase::SegLengthCatmullRom,
    &SplineBase::SegLengthBezier3,
};

SplineBase::InitMethtod SplineBase::initializers[SplineBase::ModesCount] =
{
    //&SplineOrigin::InitLinear,
    &SplineBase::InitCatmullRom,    // we should use catmullrom initializer even for linear mode! (client's internal structure limitation)
    &SplineBase::InitCatmullRom,
    &SplineBase::InitBezier3,
};

float SplineBase::SegLength( index_type Index ) const
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

void SplineBase::EvaluateLinear(index_type Idx, float u, Vector3& result) const
{
    mov_assert(Idx >= 0 && Idx+1 < points.size());
    result = points[Idx] + (points[Idx+1] - points[Idx]) * u;
}

void SplineBase::EvaluateCatmullRom( index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate(&points[Index - 1], t, s_catmullRomCoeffs, result);
}

void SplineBase::EvaluateBezier3(index_type Index, float t, Vector3& result) const
{
    Index *= 3u;
    mov_assert(Index >= 0 && Index+3 < points.size());
    C_Evaluate(&points[Index], t, s_Bezier3Coeffs, result);
}

void SplineBase::EvaluateHermiteLinear(index_type Index, float, Vector3& result) const
{
    mov_assert(Index >= 0 && Index+1 < points.size());
    result = points[Index+1] - points[Index];
}

void SplineBase::EvaluateHermiteCatmullRom(index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate_Hermite(&points[Index - 1], t, s_catmullRomCoeffs, result);
}

void SplineBase::EvaluateHermiteBezier3(index_type Index, float t, Vector3& result) const
{
    mov_assert(Index-1 >= 0 && Index+2 < points.size());
    C_Evaluate_Hermite(&points[Index - 1], t, s_Bezier3Coeffs, result);
}

float SplineBase::SegLengthLinear(index_type i) const
{
    mov_assert(i >= 0 && i+1 < points.size());
    return (points[i] - points[i+1]).length();
}

float SplineBase::SegLengthCatmullRom( index_type Index ) const
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

float SplineBase::SegLengthBezier3(index_type Index) const
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

SplineBase::SplineBase() : m_mode(ModeLinear), index_lo(0), index_hi(0), points_count(0), cyclic(false)
{
}

void SplineBase::init_spline(const Vector3 * controls, const int count, EvaluationMode m)
{
    m_mode = m;
    points_count = count;
    cyclic = false;

    (this->*initializers[m_mode])(controls, count, cyclic, 0);
}

void SplineBase::init_cyclic_spline(const Vector3 * controls, const int count, EvaluationMode m, int cyclic_point)
{
    m_mode = m;
    points_count = count;
    cyclic = true;

    (this->*initializers[m_mode])(controls, count, cyclic, cyclic_point);
}

void SplineBase::InitLinear(const Vector3* controls, const int count, bool cyclic, int cyclic_point)
{
    mov_assert(count >= 2);
    const int real_size = count + 1;

    points.resize(real_size);

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

void SplineBase::InitCatmullRom(const Vector3* controls, const int count, bool cyclic, int cyclic_point)
{
    const int real_size = count + (cyclic ? (1+2) : (1+1));

    points.resize(real_size);

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

void SplineBase::InitBezier3(const Vector3* controls, const int count, bool cyclic, int cyclic_point)
{
    index_type c = count / 3u * 3u;
    index_type t = c / 3u;

    points.resize(c);
    memcpy(&points[0],controls, sizeof(Vector3) * c);

    index_lo = 0;
    index_hi = t-1;
    //mov_assert(points.size() % 3 == 0);
}

void SplineBase::clear()
{
    index_lo = 0;
    index_hi = 0;
    points_count = 0;

    points.clear();
}

std::string SplineBase::ToString() const
{
    std::stringstream str;

    index_type count = this->points.size();
    str << "points count: " << count << std::endl;
    for (index_type i = 0; i < count; ++i)
        str << "point " << i << " : " << points[i].toString() << std::endl;

    return str.str();
}

}
