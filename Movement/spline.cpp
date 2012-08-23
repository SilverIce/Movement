
#include "spline.h"
#include <QtCore/QTextStream>
#include <G3D/Matrix4.h>

#include "spline_tests.hpp"

namespace Movement{

Q_DECLARE_TYPEINFO(G3D::Vector3, Q_MOVABLE_TYPE|Q_PRIMITIVE_TYPE);

SplineBase::EvaluationMethtod SplineBase::evaluators[SplineBase::ModeEnd] =
{
    &SplineBase::EvaluateLinear,
    &SplineBase::EvaluateCatmullRom,
};

SplineBase::EvaluationMethtod SplineBase::derivative_evaluators[SplineBase::ModeEnd] =
{
    &SplineBase::EvaluateDerivativeLinear,
    &SplineBase::EvaluateDerivativeCatmullRom,
};

SplineBase::SegLenghtMethtod SplineBase::seglengths[SplineBase::ModeEnd] =
{
    &SplineBase::SegLengthLinear,
    &SplineBase::SegLengthCatmullRom,
};

SplineBase::InitMethtod SplineBase::initializers[SplineBase::ModeEnd] =
{
    //&SplineBase::InitLinear,
    &SplineBase::InitCatmullRom,    // we should use catmullrom initializer even for linear mode! (client's internal structure limitation)
    &SplineBase::InitCatmullRom,
};

///////////
#pragma region evaluation methtods

using G3D::Matrix4;
static const Matrix4 s_catmullRomCoeffs(
    -0.5f, 1.5f,-1.5f, 0.5f,
    1.f, -2.5f, 2.f, -0.5f,
    -0.5f, 0.f,  0.5f, 0.f,
    0.f,  1.f,  0.f,  0.f);

/*  Bezier3 spline unusued
static const Matrix4 s_Bezier3Coeffs(
    -1.f,  3.f, -3.f, 1.f,
    3.f, -6.f,  3.f, 0.f,
    -3.f,  3.f,  0.f, 0.f,
    1.f,  0.f,  0.f, 0.f);*/
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

static inline void Evaluate(const Vector3 *vertice, float t, const Matrix4& matr, Vector3 &result)
{
    //Vector4 tvec(t*t*t, t*t, t, 1.f);
    //Vector4 weights(tvec * matr);
    float tvec[] = {t*t*t, t*t, t/*, 1.f*/};
    Vector4 weights(
        matr[0][0]*tvec[0] + matr[1][0]*tvec[1] + matr[2][0]*tvec[2] + matr[3][0],
        matr[0][1]*tvec[0] + matr[1][1]*tvec[1] + matr[2][1]*tvec[2] + matr[3][1],
        matr[0][2]*tvec[0] + matr[1][2]*tvec[1] + matr[2][2]*tvec[2] + matr[3][2],
        matr[0][3]*tvec[0] + matr[1][3]*tvec[1] + matr[2][3]*tvec[2] + matr[3][3]
    );

    result = vertice[0] * weights[0] + vertice[1] * weights[1]
           + vertice[2] * weights[2] + vertice[3] * weights[3];
}

static inline void Evaluate_Derivative(const Vector3 *vertice, float t, const Matrix4& matr, Vector3 &result)
{
    //Vector4 tvec(3.f*t*t, 2.f*t, 1.f, 0.f);
    //Vector4 weights(tvec * matr);
    float tvec[] = {3.f*t*t, 2.f*t/*, 1.f, 0.f*/};
    Vector4 weights(
        matr[0][0]*tvec[0] + matr[1][0]*tvec[1] + matr[2][0],
        matr[0][1]*tvec[0] + matr[1][1]*tvec[1] + matr[2][1],
        matr[0][2]*tvec[0] + matr[1][2]*tvec[1] + matr[2][2],
        matr[0][3]*tvec[0] + matr[1][3]*tvec[1] + matr[2][3]
    );

    result = vertice[0] * weights[0] + vertice[1] * weights[1]
           + vertice[2] * weights[2] + vertice[3] * weights[3];
}

void SplineBase::EvaluateLinear(index_type index, float u, Vector3& result) const
{
    mov_assert(index >= index_lo && index < index_hi);
    result = points[index] + (points[index+1] - points[index]) * u;
}

void SplineBase::EvaluateCatmullRom( index_type index, float t, Vector3& result) const
{
    mov_assert(index >= index_lo && index < index_hi);
    Evaluate(&points[index - 1], t, s_catmullRomCoeffs, result);
}

void SplineBase::EvaluateDerivativeLinear(index_type index, float, Vector3& result) const
{
    mov_assert(index >= index_lo && index < index_hi);
    result = points[index+1] - points[index];
}

void SplineBase::EvaluateDerivativeCatmullRom(index_type index, float t, Vector3& result) const
{
    mov_assert(index >= index_lo && index < index_hi);
    Evaluate_Derivative(&points[index - 1], t, s_catmullRomCoeffs, result);
}

float SplineBase::SegLengthLinear(index_type index, uint32) const
{
    mov_assert(index >= index_lo && index < index_hi);
    return (points[index] - points[index+1]).length();
}

float SplineBase::SegLengthCatmullRom(index_type index, uint32 iterationCount) const
{
    mov_assert(index >= index_lo && index < index_hi);
    mov_assert(iterationCount > 0);

    Vector3 curPos, nextPos;
    const Vector3 * p = &points[index - 1];
    curPos = nextPos = p[1];

    uint32 i = 1;
    double length = 0;
    float iterationCountInv = 1.f / (float)iterationCount;
    while (i <= iterationCount)
    {
        Evaluate(p, float(i) * iterationCountInv, s_catmullRomCoeffs, nextPos);
        length += (nextPos - curPos).length();
        curPos = nextPos;
        ++i;
    }
    return length;
}

void SplineBase::initSpline(const Vector3 * controls, index_type count, EvaluationMode m)
{
    m_mode = m;
    (this->*initializers[m_mode])(controls, count, false, 0);
}

void SplineBase::initCyclicSpline(const Vector3 * controls, index_type count, EvaluationMode m, index_type cyclic_point)
{
    m_mode = m;
    (this->*initializers[m_mode])(controls, count, true, cyclic_point);
}

void SplineBase::InitLinear(const Vector3* controls, index_type count, bool cyclic, index_type cyclic_point)
{
    mov_assert(count >= 2);
    const int real_size = count + 1;

    points.resize(real_size);

    memcpy(&points[0],controls, sizeof(Vector3) * count);

    // first and last two indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate and C_Evaluate_Derivative method work
    if (cyclic)
        points[count] = controls[cyclic_point];
    else
        points[count] = controls[count-1];

    index_lo = 0;
    index_hi = cyclic ? count : (count - 1);
}

void SplineBase::InitCatmullRom(const Vector3* controls, index_type count, bool cyclic, index_type cyclic_point)
{
    mov_assert(count >= 2);
    const int real_size = count + (cyclic ? (1+2) : (1+1));

    points.resize(real_size);

    int lo_index = 1;
    int high_index = lo_index + count - 1;

    memcpy(&points[lo_index],controls, sizeof(Vector3) * count);

    // first and last two indexes are space for special 'virtual points'
    // these points are required for proper C_Evaluate and C_Evaluate_Derivative method work
    if (cyclic)
    {
        if (cyclic_point == 0)
            points[0] = controls[count-1];
        else
            points[0] = controls[0].lerp(controls[1], -1);

        points[high_index+1] = controls[cyclic_point];
        points[high_index+2] = controls[cyclic_point+1];
    }
    else
    {
        points[0] = controls[0].lerp(controls[1], -1);
        points[high_index+1] = controls[count-1];
    }

    index_lo = lo_index;
    index_hi = high_index + (cyclic ? 1 : 0);
}

QString SplineBase::toString() const
{
    QString string;
    QTextStream str(&string);
    const char * mode_str[ModeEnd] = {"Linear", "CatmullRom"};

    index_type count = this->points.size();
    str << endl << "mode: " << mode_str[mode()];
    str << endl << "points count: " << count;
    for (index_type i = 0; i < count; ++i)
        str << endl << "point " << (i - index_lo) << " : " << points[i].toString().c_str();

    return *str.string();
}

}
