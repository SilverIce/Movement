#include "framework/gtest.h"

namespace Movement
{
    using G3D::fuzzyEq;

    void testforNaN(testing::State& testState, const Spline<float>& spline)
    {
        enum { cycles = 1000 };

        for (uint32 I = 0; I < (cycles-1); ++I)
        {
            float t = I / (float)cycles;

            Vector3 point = spline.evaluatePosition(t);
            EXPECT_TRUE( point.isFinite() );

            Vector3 direction = spline.evaluateDerivative(t).direction();
            EXPECT_TRUE( direction.isFinite() );
        }
    }

    void testDerivation(testing::State& testState, const Spline<float>& spline)
    {
        // Linear spline is not smooth to pass this test successfully:
        if (spline.mode() == SplineBase::ModeLinear)
            return;

        enum { cycles = 1000 };
        for (uint32 I = 0; I < (cycles-1); ++I)
        {
            float t = I / (float)cycles;

            Vector3 point = spline.evaluatePosition(t);
            EXPECT_TRUE( point.isFinite() );

            Vector3 direction = spline.evaluateDerivative(t).direction();
            EXPECT_TRUE( direction.isFinite() );

            Vector3 point2 = spline.evaluatePosition(t + 1 / (float)cycles);
            Vector3 direction2 = (point2-point).direction();
            // angleDiff is angle between direction and direction2 unit vectors
            float angleDiff = G3D::toDegrees(2*asin((direction-direction2).length()/2));
            EXPECT_TRUE( angleDiff < 0.7f );
        }
    }

    TEST(SplineTests, basic)
    {
        const Vector3 points[] = {
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
        splines[0].initSpline(points, CountOf(points), SplineBase::ModeLinear);
        splines[1].initCyclicSpline(points, CountOf(points), SplineBase::ModeLinear, 0);
        splines[2].initSpline(points, CountOf(points), SplineBase::ModeCatmullrom);
        splines[3].initCyclicSpline(points, CountOf(points), SplineBase::ModeCatmullrom, 0);

        for (int i = 0; i < CountOf(splines); ++i)
            splines[i].initLengths();

        const float properLengths[4] = {
            253.202179f,
            292.727539f,
            256.882568f,
            298.148926f,
        };

        for (int i = 0; i < CountOf(splines); ++i)
        {
            Spline<float>& spline = splines[i];
            EXPECT_TRUE( G3D::fuzzyEq(spline.lengthTotal(),properLengths[i]) );

            testforNaN(testState, spline);
            testDerivation(testState, spline);

            // ensures that cached segment length is equal to computed
            for (int idx = 1; idx < spline.last(); idx++)
                EXPECT_TRUE( fuzzyEq(spline.lengthBetween(idx-1,idx), spline.segmentLength(idx-1)) );

            // generic linear and catmullrom spline property:
            // point coords at beginning of the spline segment(and at the end) are equal to path point coords 
            for (int pointIdx = 0; pointIdx < (CountOf(points)-1); ++pointIdx)
                EXPECT_TRUE( points[pointIdx].fuzzyEq(spline.evaluatePosition(pointIdx,0)) );

            for (float t = 0; t <= 1; t += 0.1f)
            {
                int32 splineIdx = spline.computeIndexInBounds(t);
                EXPECT_TRUE( spline.length(splineIdx) <= t*spline.lengthTotal() );
                EXPECT_TRUE( t*spline.lengthTotal() < spline.length(splineIdx+1) );
            }
        }
    }

    TEST(SplineTests, uninitialized)
    {
        SplineBase spline;

        EXPECT_THROW(spline.getPoint(2), ARGS(Exception<SplineBase,SplineBase::Uninitialized>));
        EXPECT_THROW(spline.segmentLength(2), ARGS(Exception<SplineBase,SplineBase::Uninitialized>));
        EXPECT_THROW(spline.evaluatePosition(1,0), ARGS(Exception<SplineBase,SplineBase::Uninitialized>));
        EXPECT_THROW(spline.evaluateDerivative(4, 0), ARGS(Exception<SplineBase,SplineBase::Uninitialized>));

        typedef Exception<SplineBase,SplineBase::InitializationFailed> ExcSplineInitFailed;
        SplineBase::ControlArray points;
        EXPECT_THROW(spline.initSpline(points.constData(),points.count(),SplineBase::ModeLinear), ExcSplineInitFailed);
        points.resize(2);
        EXPECT_NOTHROW(spline.initSpline(points.constData(),points.count(),SplineBase::ModeLinear));
        EXPECT_THROW(spline.initSpline(nullptr,points.count(),SplineBase::ModeLinear), ExcSplineInitFailed);
        EXPECT_THROW(spline.initSpline(points.constData(),points.count(),SplineBase::ModeEnd), ExcSplineInitFailed);
        EXPECT_THROW(spline.initCyclicSpline(points.constData(),points.count(),SplineBase::ModeCatmullrom,points.count()), ExcSplineInitFailed);
    }
}
