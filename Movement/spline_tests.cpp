#include "spline.h"

namespace Movement
{
    void testforNaN(const Spline<float>& spline)
    {
        for (float t = 0.f; t < 1.f; t += 0.001f)
        {
            Vector3 point;
            spline.evaluate_percent(t, point);
            check( point.isFinite() );

            Vector3 direction;
            spline.evaluate_derivative(t, direction);
            check( direction.isFinite() );
        }
    }

    void SplineBasicTest()
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

        Spline<float> spline[4];
        spline[0].init_spline(nodes, CountOf(nodes), SplineBase::ModeLinear);
        spline[1].init_cyclic_spline(nodes, CountOf(nodes), SplineBase::ModeLinear, 0);
        spline[2].init_spline(nodes, CountOf(nodes), SplineBase::ModeCatmullrom);
        spline[3].init_cyclic_spline(nodes, CountOf(nodes), SplineBase::ModeCatmullrom, 0);
        for (int i = 0; i < CountOf(spline); ++i)
            spline[i].initLengths();

        for (int i = 0; i < CountOf(spline); ++i)
            testforNaN(spline[i]);
    }

    void SplineTests()
    {
        SplineBasicTest();
    }
}
