#include "framework/gtest.h"

namespace Movement
{
    void testforNaN(const Spline<float>& spline)
    {
        for (float t = 0.f; t < 1.f; t += 0.001f)
        {
            Vector3 point;
            spline.evaluatePosition(t, point);
            EXPECT_TRUE( point.isFinite() );

            Vector3 direction;
            spline.evaluateDerivative(t, direction);
            EXPECT_TRUE( direction.isFinite() );
        }
    }


    TEST(SplineTest, BasicTest)
    {
        Vector3 nodes[] = {
            Vector3(-4000.046f, 985.8019f, 61.02531f),
            Vector3(-3981.982f, 1017.846f, 58.96975f),
            Vector3(-3949.962f, 1033.053f, 56.85864f),
            Vector3(-3918.825f, 1014.746f, 58.33086f),
            Vector3(-3900.323f, 984.7424f, 60.60864f),
            Vector3(-3918.999f, 953.8466f, 58.96975f),
            Vector3(-3950.793f, 934.2088f, 58.96975f),
            Vector3(-3982.866f, 950.2649f, 58.96975f),
        };

        Spline<float> splines[4];
        splines[0].initSpline(nodes, CountOf(nodes), SplineBase::ModeLinear);
        splines[1].initCyclicSpline(nodes, CountOf(nodes), SplineBase::ModeLinear, 0);
        splines[2].initSpline(nodes, CountOf(nodes), SplineBase::ModeCatmullrom);
        splines[3].initCyclicSpline(nodes, CountOf(nodes), SplineBase::ModeCatmullrom, 0);

        const float properLengths[4] = {
            253.202179f,
            292.727539f,
            256.882568f,
            298.148926f,
        };

        for (int i = 0; i < CountOf(splines); ++i)
        {
            Spline<float>& spline = splines[i];

            spline.initLengths();
            EXPECT_TRUE( G3D::fuzzyEq(spline.lengthTotal(),properLengths[i]) );

            testforNaN(spline);

            for (int pointIdx = 0; pointIdx < CountOf(nodes); ++pointIdx) {
                if ((spline.first() + pointIdx) < spline.last()) {
                    Vector3 calculated;
                    spline.evaluatePosition(spline.first() + pointIdx, 0.f, calculated);
                    EXPECT_TRUE( calculated.fuzzyEq(nodes[pointIdx]) );
                }
            }
        }
    }
}
