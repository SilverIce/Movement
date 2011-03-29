#pragma once
#include "G3D/Vector3.h"

namespace Movement
{
    using G3D::Vector3;

    struct Location : public Vector3
    {
        Location() : orientation(0) {}
        Location(float x, float y, float z, float o) : Vector3(x,y,z), orientation(o) {}
        Location(const Vector3& v) : Vector3(v), orientation(0) {}

        float orientation;
    };
}
