#include "framework/gtest.h"

namespace Movement
{
    void testforNaN(const Spline<float>& spline)
    {
        float maxEr = 0;
        enum { cycles = 1000 };

        for (uint32 I = 0; I < (cycles-1); ++I)
        {
            float t = I / (float)cycles;

            Vector3 point = spline.evaluatePosition(t);
            EXPECT_TRUE( point.isFinite() );

            Vector3 direction = spline.evaluateDerivative(t).direction();
            EXPECT_TRUE( direction.isFinite() );

            // Linear spline is not smooth to pass this test successfully:
            if (spline.mode() != SplineBase::ModeLinear) {
                Vector3 point2 = spline.evaluatePosition(t + 1 / (float)cycles);
                Vector3 direction2 = (point2-point).direction();
                // angleDiff is angle between direction and direction2 unit vectors
                float angleDiff = G3D::toDegrees(2*asin((direction-direction2).length()/2));
                EXPECT_TRUE( angleDiff < 0.7f );
            }
        }
    }

    TEST(SplineTest, BasicTest)
    {
        const Vector3 nodes[] = {
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

        for (int i = 0; i < CountOf(splines); ++i)
            splines[i].initLengths(3);

        const float properLengths[4] = {
            253.202179f,
            292.727539f,
            256.882568f,
            298.148926f,
        };

        for (int i = 0; i < CountOf(splines); ++i) {
            Spline<float>& spline = splines[i];
            EXPECT_TRUE( G3D::fuzzyEq(spline.lengthTotal(),properLengths[i]) );
            testforNaN(spline);
        }
    }
}
